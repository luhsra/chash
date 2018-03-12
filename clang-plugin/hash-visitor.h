//===--- CHashVisitor.h - Stable AST Hashing -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the CHashVisitor class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_CHASHVISITOR_H
#define LLVM_CLANG_AST_CHASHVISITOR_H

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DataCollection.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/MD5.h"
#include <map>
#include <string>

namespace clang {


template <typename H = llvm::MD5, typename HR = llvm::MD5::MD5Result>
class CHashVisitor : public clang::RecursiveASTVisitor<CHashVisitor<H, HR>> {

  using Inherited = clang::RecursiveASTVisitor<CHashVisitor<H, HR>>;

public:
  using Hash = H;
  using HashResult = HR;

  /// Configure the RecursiveASTVisitor
  bool shouldWalkTypesOfTypeLocs() const { return false; }

protected:
  ASTContext &Context;

  // For the DataCollector, we implement a few addData() functions
  void addData(uint64_t data) { topHash().update(data); }
  void addData(const StringRef &str) { topHash().update(str); }
  // On our way down, we meet a lot of qualified types.
  void addData(const QualType &T) {
    // 1. Hash referenced type
    const Type *const ActualType = T.getTypePtr();
    assert(ActualType != nullptr);

    // FIXME: Structural hash
    // 1.1 Was it already hashed?
    const HashResult *const SavedDigest = getHash(ActualType);
    if (SavedDigest) {
      // 1.1.1 Use cached value
      topHash().update(SavedDigest->Bytes);
    } else {
      // 1.1.2 Calculate hash for type
      const Hash *const CurrentHash = pushHash();
      Inherited::TraverseType(T); // Uses getTypePtr() internally
      const HashResult TypeDigest = popHash(CurrentHash);
      topHash().update(TypeDigest.Bytes);

      // Store hash for underlying type
      storeHash(ActualType, TypeDigest);
    }

    // Add the qulaifiers at this specific usage of the type
    addData(T.getCVRQualifiers());
  }

  /// The RecursiveASTVisitor often visits several children of a node
  /// in a for loop. However, we also have to include the number of
  /// children into the hash in order to avoid collisions. Therefore,
  /// the following addData(..) methods only include the lengths into
  /// the hash. The actual children are visited by the
  /// RecursiveASTVistor.
  template<typename T>
  void addData(const llvm::iterator_range<T> &x) {
    addData(std::distance(x.begin(), x.end()));
  }
  template<typename T>
  void addData(const llvm::ArrayRef<T> &x) {
    addData(x.size());
  }

public:
#define DEF_ADD_DATA_STORED(CLASS, CODE)                                       \
  template <class = void> bool Visit##CLASS(const CLASS *S) {                  \
    CODE;                                                                      \
    return true;                                                               \
  }
#define DEF_ADD_DATA(CLASS, CODE) DEF_ADD_DATA_STORED(CLASS, CODE)
#include "clang/AST/StmtDataCollectors.inc"
#define DEF_ADD_DATA(CLASS, CODE) DEF_ADD_DATA_STORED(CLASS, CODE)
#include "clang/AST/AttrDataCollectors.inc"
#define DEF_ADD_DATA(CLASS, CODE) DEF_ADD_DATA_STORED(CLASS, CODE)
#include "clang/AST/DeclDataCollectors.inc"
#define DEF_ADD_DATA(CLASS, CODE) DEF_ADD_DATA_STORED(CLASS, CODE)
#include "clang/AST/TypeDataCollectors.inc"

  CHashVisitor(ASTContext &Context) : Context(Context) {}

  /// For some special nodes, override the traverse function, since
  /// we need both pre- and post order traversal
  bool TraverseTranslationUnitDecl(TranslationUnitDecl *TU) {
    if (!TU)
      return true;
    // First, we push a new hash onto the hashing stack. This hash
    // will capture everythin within the TU*/
    Hash *CurrentHash = pushHash();

    Inherited::WalkUpFromTranslationUnitDecl(TU);

    // Do recursion on our own, since we want to exclude some children
    const auto DC = cast<DeclContext>(TU);
    for (auto *Child : DC->noload_decls()) {
      if (isa<TypedefDecl>(Child) || isa<RecordDecl>(Child) ||
          isa<EnumDecl>(Child))
        continue;

      // Extern variable definitions at the top-level
      if (const auto VD = dyn_cast<VarDecl>(Child)) {
        if (VD->hasExternalStorage()) {
          continue;
        }
      }

      if (const auto FD = dyn_cast<FunctionDecl>(Child)) {
        // We try to avoid hashing of declarations that have no definition
        if (!FD->isThisDeclarationADefinition()) {
          bool doHashing = false;
          // HOWEVER! If this declaration is an alias Declaration, we
          // hash it no matter what
          if (FD->hasAttrs()) {
            for (const Attr *const A : FD->getAttrs()) {
              if (A->getKind() == attr::Kind::Alias) {
                doHashing = true;
                break;
              }
            }
          }
          if (!doHashing)
            continue;
        }
      }

      TraverseDecl(Child);
    }

    storeHash(TU, popHash(CurrentHash));

    return true;
  }

  bool TraverseDecl(Decl *D) {
    if (!D)
      return true;
    /* For some declarations, we store the calculated hash value. */
    bool CacheHash = false;
    if (isa<FunctionDecl>(D) && cast<FunctionDecl>(D)->isDefined())
      CacheHash = true;
    if (isa<VarDecl>(D) && cast<VarDecl>(D)->hasGlobalStorage())
      CacheHash = true;
    if (isa<RecordDecl>(D) && dyn_cast<RecordDecl>(D)->isCompleteDefinition())
      CacheHash = true;

    if (!CacheHash) {
      return Inherited::TraverseDecl(D);
    }

    const HashResult *const SavedDigest = getHash(D);
    if (SavedDigest) {
      topHash().update(SavedDigest->Bytes);
      return true;
    }
    Hash *CurrentHash = pushHash();
    bool Ret = Inherited::TraverseDecl(D);
    HashResult CurrentHashResult = popHash(CurrentHash);
    storeHash(D, CurrentHashResult);
    if (!isa<TranslationUnitDecl>(D)) {
      topHash().update(CurrentHashResult.Bytes);
    }

    return Ret;
  }

  /// When doing a semantic hash, we have to use cross-tree links to
  /// other parts of the AST, here we establish these links

#define DEF_TYPE_GOTO_DECL(CLASS, EXPR)                                 \
  bool Visit##CLASS(CLASS *T) {                                                \
    Inherited::Visit##CLASS(T);                                                \
    return TraverseDecl(EXPR);                                                 \
  }

  DEF_TYPE_GOTO_DECL(TypedefType, T->getDecl());
  DEF_TYPE_GOTO_DECL(RecordType, T->getDecl());
  // The EnumType forwards to the declaration. The declaration does
  // not hand back to the type.
  DEF_TYPE_GOTO_DECL(EnumType, T->getDecl());
  bool TraverseEnumDecl(EnumDecl *E) {
    /* In the original RecursiveASTVisitor
       > if (D->getTypeForDecl()) {
       >    TRY_TO(TraverseType(QualType(D->getTypeForDecl(), 0)));
       > }
       => NO, NO, NO, to avoid endless recursion
    */
    return Inherited::WalkUpFromEnumDecl(E);
  }

  bool VisitTypeDecl(TypeDecl *D) {
    // If we would hash the resulting type for a typedef, we
    // would get into an endless recursion.
    if (!isa<TypedefNameDecl>(D) && !isa<RecordDecl>(D) && !isa<EnumDecl>(D)) {
      addData(QualType(D->getTypeForDecl(), 0));
    }
    return true;
  }

  bool VisitDeclRefExpr(DeclRefExpr *E) {
    ValueDecl *ValDecl = E->getDecl();
    // Function Declarations are handled in VisitCallExpr
    if (!ValDecl) {
      return true;
    }
    if (isa<VarDecl>(ValDecl)) {
      /* We emulate TraverseDecl here for VarDecl, because we
       * are not allowed to call TraverseDecl here, since the
       * initial expression of a DeclRefExpr might reference a
       * sourronding Declaration itself. For example:
       *
       * struct foo {int N;}
       * struct foo a = { sizeof(a) };
       */
      VarDecl *VD = static_cast<VarDecl *>(ValDecl);
      VisitNamedDecl(VD);
      Inherited::TraverseType(VD->getType());
      VisitVarDecl(VD);
    } else if (isa<FunctionDecl>(ValDecl)) {
      /* Hash Functions without their body */
      FunctionDecl *FD = static_cast<FunctionDecl *>(ValDecl);
      Stmt *Body = FD->getBody();
      FD->setBody(nullptr);
      TraverseDecl(FD);
      FD->setBody(Body);
    } else {
      TraverseDecl(ValDecl);
    }
    return true;
  }

  bool VisitValueDecl(ValueDecl *D) {
    /* Field Declarations can induce recursions */
    if (isa<FieldDecl>(D)) {
      addData(std::string(D->getType().getAsString()));
    } else {
      addData(D->getType());
    }
    addData(D->isWeak());
    return true;
  }

  bool VisitExpr(Expr *E) {
    // We specially treat the CXXThisExpr, as we cannot call
    // addData(QualType) directly on this, because it would reference
    // back to the enclosing CXXRecord and result in a recursion.
    if (isa<CXXThisExpr>(E)) {
      addData(std::string(E->getType().getAsString()));
    } else {
      Inherited::VisitExpr(E);
    }
    return true;
  }

  /// For performance reasons, we cache some of the hashes for types
  /// and declarations.

public:
  // We store hashes for declarations and types in separate maps.
  std::map<const Type *, HashResult> TypeSilo;
  std::map<const Decl *, HashResult> DeclSilo;

  void storeHash(const Type *Obj, HashResult Dig) { TypeSilo[Obj] = Dig; }

  void storeHash(const Decl *Obj, HashResult Dig) { DeclSilo[Obj] = Dig; }

  const HashResult *getHash(const Type *Obj) {
    if (TypeSilo.find(Obj) != TypeSilo.end()) {
      return &TypeSilo[Obj];
    }
    return nullptr;
  }

  const HashResult *getHash(const Decl *Obj) {
    if (DeclSilo.find(Obj) != DeclSilo.end()) {
      return &DeclSilo[Obj];
    }
    return nullptr;
  }

  /// In order to produce hashes for subtrees on the way, a hash
  /// stack is used. When a new subhash is meant to be calculated,
  /// we push a new stack on the hash. All hashing functions use
  /// always the top of the hashing stack.

protected:
  llvm::SmallVector<Hash, 32> HashStack;

public:
  Hash *pushHash() {
    HashStack.push_back(Hash());
    return &HashStack.back();
  }

  HashResult popHash(const Hash *ShouldBe = nullptr) {
    assert(!ShouldBe || ShouldBe == &HashStack.back());

    // Finalize the Hash and return the digest.
    HashResult CurrentDigest;
    topHash().final(CurrentDigest);
    HashStack.pop_back();
    return CurrentDigest;
  }

  Hash &topHash() { return HashStack.back(); }
};

} // namespace clang
#endif // LLVM_CLANG_AST_CHASHVISITOR_H
