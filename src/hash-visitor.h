#ifndef __HASH_VISITOR
#define __HASH_VISITOR

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/AST/TypeVisitor.h"
#include <string>
#include <map>
#include <vector>
#include <tuple>

#include "Hash.h"

using namespace clang;

class TranslationUnitHashVisitor
    : public ConstDeclVisitor<TranslationUnitHashVisitor, bool>,
      public ConstStmtVisitor<TranslationUnitHashVisitor, bool>,
      public TypeVisitor<TranslationUnitHashVisitor, bool> {

  typedef ConstDeclVisitor<TranslationUnitHashVisitor, bool> mt_declvisitor;
  typedef ConstStmtVisitor<TranslationUnitHashVisitor, bool> mt_stmtvisitor;
  typedef TypeVisitor<TranslationUnitHashVisitor, bool> mt_typevisitor;

  Hash TopLevelHash;

  // In this storage we save hashes for various memory objects
  std::map<const void *, Hash::Digest> Silo;

  // /// Pending[i] is an action to hash an entity at level i.
  bool FirstChild;
  llvm::SmallVector<std::function<void()>, 32> Pending;

  /// Hash a child of the current node.
  unsigned beforeDescent() {
    FirstChild = true;
    return Pending.size();
  }

  template <typename Fn> void afterChildren(Fn Func) {
    if (FirstChild) {
      Pending.push_back(std::move(Func));
    } else {
      Pending.back()();
      Pending.back() = std::move(Func);
    }
    FirstChild = false;
  }

  void afterDescent(unsigned Depth) {
    // If any children are left, they're the last at their nesting level.
    // Hash those ones out now.
    while (Depth < Pending.size()) {
      Pending.back()();
      this->Pending.pop_back();
    }
  }

  llvm::SmallVector<Hash, 32> HashStack;

public:
  // Utilities
  bool hasNodes(const DeclContext *DC);
  void hashDeclContext(const DeclContext *DC);

  void hashDecl(const Decl *);
  void hashStmt(const Stmt *);
  void hashType(QualType);
  void hashAttr(const Attr *);

  void hashName(const NamedDecl *);

  // C Declarations
  bool VisitTranslationUnitDecl(const TranslationUnitDecl *);
  bool VisitVarDecl(const VarDecl *);
  /// Not interesting
  bool VisitTypedefDecl(const TypedefDecl *) { return true; };

  /* Wird erst in Aufrufen geprueft */
  bool VisitRecordDecl(const RecordDecl *D) { return true; };
  bool VisitFieldDecl(const FieldDecl *D) { return true; };

  // C Types
  bool VisitBuiltinType(const BuiltinType *);
  bool VisitPointerType(const PointerType *T);
  bool VisitArrayType(const ArrayType *T);
  bool VisitConstantArrayType(const ConstantArrayType *T);
  bool VisitVariableArrayType(const VariableArrayType *T);
  bool VisitType(const Type *T);
  bool VisitTypedefType(const TypedefType *T);
  bool VisitComplexType(const ComplexType *T);
  bool VisitAtomicType(const AtomicType *T);
  bool VisitTypeOfExprType(const TypeOfExprType *T);
  bool VisitTypeOfType(const TypeOfType *T);
  bool VisitParenType(const ParenType *T);
  bool VisitFunctionType(const FunctionType *T);
  bool VisitFunctionProtoType(const FunctionProtoType *T);
  bool VisitEnumType(const EnumType *Node);
  bool VisitTagType(const TagType *Node);
  bool VisitAttributedType(const AttributedType *Node);
  bool VisitUnaryTransformType(const UnaryTransformType *T);
  bool VisitDecayedType(const DecayedType *T);
  bool VisitAdjustedType(const AdjustedType *T);
  bool VisitElaboratedType(const ElaboratedType *T);

  std::string getHash(unsigned *ProcessedBytes = nullptr);

  // C Exprs (no clang-builtins, ...)
  /*bool VisitExpr(const Expr *Node);*/
  bool VisitCastExpr(const CastExpr *Node);
  bool VisitDeclRefExpr(const DeclRefExpr *Node);
  bool VisitPredefinedExpr(const PredefinedExpr *Node);
  bool VisitCharacterLiteral(const CharacterLiteral *Node);
  bool VisitIntegerLiteral(const IntegerLiteral *Node);
  bool VisitFloatingLiteral(const FloatingLiteral *Node);
  bool VisitStringLiteral(const StringLiteral *Str);
  bool VisitInitListExpr(const InitListExpr *ILE);
  bool VisitUnaryOperator(const UnaryOperator *Node);
  bool VisitUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *Node);
  bool VisitMemberExpr(const MemberExpr *Node);
  bool VisitBinaryOperator(const BinaryOperator *Node);
  bool VisitCompoundAssignOperator(const CompoundAssignOperator *Node);
  bool VisitAddrLabelExpr(const AddrLabelExpr *Node);
  bool VisitBlockExpr(const BlockExpr *Node);
  bool VisitArraySubscriptExpr(const ArraySubscriptExpr *Node);
  bool VisitImplicitValueInitExpr(const ImplicitValueInitExpr *Node);
  bool VisitCompoundLiteralExpr(const CompoundLiteralExpr *Node);
  bool VisitImaginaryLiteral(const ImaginaryLiteral *Node);
  bool
  VisitAbstractConditionalOperator(const AbstractConditionalOperator *Node);
  bool VisitBinaryConditionalOperator(const BinaryConditionalOperator *Node);
  bool VisitCallExpr(const CallExpr *Node);
  bool VisitOffsetOfExpr(const OffsetOfExpr *Node);
  bool VisitParenExpr(const ParenExpr *Node);
  bool VisitAtomicExpr(const AtomicExpr *Node);
  bool VisitParenListExpr(const ParenListExpr *Node);
  bool VisitDesignatedInitExpr(const DesignatedInitExpr *Node);
  bool VisitStmtExpr(const StmtExpr *Node);
  bool VisitVAArgExpr(const VAArgExpr *Node);

  // TODO: evtl. ImplicitValueInitExpr, GenericSelectionExpr, ArraySubscriptExpr
  // TODO: evtl. OpaqueValueExpr, ExtVectorElementExpr (Beschreibung klingt nach
  // C++)

  // declarations
  bool VisitFunctionDecl(const FunctionDecl *D);
  bool VisitBlockDecl(const BlockDecl *Node);
  bool VisitLabelDecl(const LabelDecl *Node);
  bool VisitEnumDecl(const EnumDecl *Node);
  bool VisitEnumConstantDecl(const EnumConstantDecl *Node);
  bool VisitImplicitParamDecl(const ImplicitParamDecl *Node);
  bool VisitParmVarDecl(const ParmVarDecl *Node);
  // DeclaratorDecl done...
  bool VisitIndirectFieldDecl(const IndirectFieldDecl *Node);
  bool VisitValueDecl(const ValueDecl *Node); // maybe called by children
  bool VisitFileScopeAsmDecl(const FileScopeAsmDecl *Node);
  bool VisitCapturedDecl(const CapturedDecl *Node);

  // Attrs
  // uncommented Attr not found in namespace
  bool VisitAttr(const Attr *attr);
  bool VisitInheritableAttr(const InheritableAttr *attr);
  // bool VisitStmtAttr(const StmtAttr *attr);
  bool VisitInheritableParamAttr(const InheritableParamAttr *attr);
  // bool VisitParameterABIAttr(const ParameterABIAttr *attr);

  // statements
  bool VisitStmt(const Stmt *Node);
  bool VisitCompoundStmt(const CompoundStmt *Stmt);
  bool VisitBreakStmt(const BreakStmt *Stmt);
  bool VisitContinueStmt(const ContinueStmt *Stmt);
  bool VisitGotoStmt(const GotoStmt *Stmt);
  bool VisitLabelStmt(const LabelStmt *Stmt);
  bool VisitDoStmt(const DoStmt *Stmt);
  bool VisitForStmt(const ForStmt *Stmt);
  bool VisitIfStmt(const IfStmt *Stmt);
  bool VisitNullStmt(const NullStmt *Stmt);
  bool VisitReturnStmt(const ReturnStmt *Stmt);
  bool VisitWhileStmt(const WhileStmt *Stmt);
  bool VisitSwitchStmt(const SwitchStmt *Stmt);
  bool VisitCaseStmt(const CaseStmt *Stmt);
  bool VisitDefaultStmt(const DefaultStmt *Stmt);
  bool VisitDeclStmt(const DeclStmt *Stmt);
  bool VisitGCCAsmStmt(const GCCAsmStmt *Stmt);
  bool VisitMSAsmStmt(const MSAsmStmt *Stmt);

  // not sure if we need this
  bool VisitAttributedStmt(const AttributedStmt *Stmt);
  bool VisitCapturedStmt(const CapturedStmt *Stmt);
  bool VisitSEHExceptStmt(const SEHExceptStmt *Stmt);
  bool VisitSEHFinallyStmt(const SEHFinallyStmt *Stmt);
  bool VisitSEHLeaveStmt(const SEHLeaveStmt *Stmt);
  bool VisitSEHTryStmt(const SEHTryStmt *Stmt);
  bool VisitIndirectGotoStmt(const IndirectGotoStmt *Stmt);

  // not implemented
  bool VisitOMPExecutableDirective(const OMPExecutableDirective *Stmt);

protected:
  enum AstElementPrefix {
    AstElementVarDecl = 0xb19c2ee2,
    AstElementVarDecl_init = 0x66734486,
    AstElementImplicitParamDecl = 0xd04f138f,
    AstElementParamVarDecl = 0x1fe2fcb9,

    AstElementStmtExpr = 0xf4bb377e,
    AstElementCastExpr = 0x7c505e88,

    AstElementCharacterLiteral = 0x2a1c033f,
    AstElementIntegerLiteral = 0x7b2daa87,
    AstElementFloatingLiteral = 0xceee8473,
    AstElementStringLiteral = 0xe5846c45,

    AstElementForStmt = 0xec4e334f,
    AstElementIfStmt = 0x3de06c3c,
    AstElementNullStmt = 0x777400e0,
    AstElementDoStmt = 0xa80405bd,
    AstElementGotoStmt = 0xec2a6be8,
    AstElementContinueStmt = 0x2c518360,
    AstElementReturnStmt = 0x1cf8354e,
    AstElementWhileStmt = 0x6cb85f96,
    AstElementLabelStmt = 0xe3d17613,
    AstElementSwitchStmt = 0x6ef423db,
    AstElementCaseStmt = 0x9640cc21,
    AstElementDefaultStmt = 0x2f6febe9,
    AstElementDeclStmt = 0xbe748556,

    // AstElement = 0x5b868718
    // AstElement = 0xd0b37bef
    // AstElement = 0x6439c9ef
    // AstElement = 0x74887cd4
    // AstElement = 0x75d5304a
    // AstElement = 0x8a024d89
    // AstElement = 0x3417cfda
    // AstElement = 0x98090139
    // AstElement = 0x7c2df2fc
    // AstElement = 0x8647819b
    // AstElement = 0x4dd5f204
    // AstElement = 0x4acd4cde
    // AstElement = 0x94c7a399
    // AstElement = 0xddc8426
    // AstElement = 0xca8afa5b
    // AstElement = 0x707c703e
    // AstElement = 0x9936193
    // AstElement = 0x96681107
    // AstElement = 0xa33a24f3
    // AstElement = 0xffb3cc20
    // AstElement = 0xe23aaddd
    // AstElement = 0x496a1fb5
    // AstElement = 0xb4995380
    // AstElement = 0xe682fc67
    // AstElement = 0xe511b92e
    // AstElement = 0xc54ffefa
    // AstElement = 0x427cc6e8
    // AstElement = 0x48232f36
    // AstElement = 0xf1a9c911
    // AstElement = 0x7e5497b7
    // AstElement = 0x64600f
    // AstElement = 0x8d017154
    // AstElement = 0x6cf40f99
    // AstElement = 0x8c7ab6b2
    // AstElement = 0xfe7647fa
    // AstElement = 0xdf10fedc
    // AstElement = 0xcc75aacd
    // AstElement = 0x761e230f
    // AstElement = 0x2a34b689
    // AstElement = 0xff6db781
    // AstElement = 0xc564aed1
    // AstElement = 0x11050d85
    // AstElement = 0x937408ea
    // AstElement = 0xbb06d011
    // AstElement = 0x381879fa
    // AstElement = 0xa3a884ed
    // AstElement = 0x56b6cba9
    // AstElement = 0x7c0b04ce
    // AstElement = 0x6a4fdb90
    // AstElement = 0x906b6fb4
    // AstElement = 0x530ae0a9
    // AstElement = 0x652782d6
    // AstElement = 0xccd123ef
    // AstElement = 0x8e36d148
    // AstElement = 0x1cafe3db
    // AstElement = 0x98888356
    // AstElement = 0xa5b0d36d
    // AstElement = 0x5057c896
    // AstElement = 0xa6339d46
    // AstElement = 0x9c582bf3
    // AstElement = 0x151982b7
    // AstElement = 0x40d2aa93
    // AstElement = 0xe340180e
    // AstElement = 0x17f2d532
    // AstElement = 0xac0c83d4
    // AstElement = 0x27892cea
    // AstElement = 0x4ed393c3
    // AstElement = 0x2e2321ad
    // AstElement = 0xfe447195
    // AstElement = 0xe9bda7ae
    // AstElement = 0x366466fc
    // AstElement = 0xd6b02f4e
    // AstElement = 0x9b31edf6
    // AstElement = 0x45b2a746
    // AstElement = 0xc853e2ac
    // AstElement = 0xbcfa92ec
    // AstElement = 0x1cc7935
  };

  bool DoNotHashThis = false; // Flag used to ignore Nodes such as extern Decls
  std::map<const void *, const void *> SeenTypes;

  bool haveSeen(const void *key, const void *type) {
    if (SeenTypes.find(key) != SeenTypes.end()) {
      return true;
    }
    SeenTypes[key] = type;
    return false;
  }

  bool dummyFunctionDecl(FunctionDecl *fd) {
    // Ignore extern declarations
    if (fd->getStorageClass() == StorageClass::SC_Extern ||
        fd->getStorageClass() == StorageClass::SC_PrivateExtern) {
      return true;
    }

    Hash() << "FunctionDecl";
    Hash() << fd->getNameInfo().getName().getAsString();

    Hash() << fd->isDefined();
    Hash() << fd->isThisDeclarationADefinition();
    Hash() << fd->isVariadic();
    Hash() << fd->isVirtualAsWritten();
    Hash() << fd->isPure();
    Hash() << fd->hasImplicitReturnZero();
    Hash() << fd->hasPrototype();
    Hash() << fd->hasWrittenPrototype();
    Hash() << fd->hasInheritedPrototype();
    Hash() << fd->isMain();
    Hash() << fd->isExternC();
    Hash() << fd->isGlobal();
    Hash() << fd->isNoReturn();
    Hash() << fd->hasSkippedBody(); //???
    Hash() << fd->getBuiltinID();

    Hash() << fd->getStorageClass(); // static and stuff
    Hash() << fd->isInlineSpecified();
    Hash() << fd->isInlined();

    // hash all parameters
    for (ParmVarDecl *decl : fd->parameters()) {
      hashDecl(decl);
    }

    // vielleicht will man das ja auch:
    for (NamedDecl *decl : fd->getDeclsInPrototypeScope()) {
      hashDecl(decl);
    }

    // visit QualType
    hashType(fd->getReturnType());

    // here an error (propably nullptr) occured
    const IdentifierInfo *ident = fd->getLiteralIdentifier();
    if (ident != nullptr) {
      const char *str = ident->getNameStart();
      if (str != nullptr)
        Hash() << str;
    }

    Hash() << fd->getMemoryFunctionKind(); // maybe needed
    return true;
  }

  void dummyVarDecl(VarDecl *vd) {
    hashName(vd);
    hashType(vd->getType());
    Hash() << vd->getStorageClass();
    Hash() << vd->getTLSKind();
    Hash() << vd->isModulePrivate();
    Hash() << vd->isNRVOVariable();
  }

  // Hash Silo
  void storeHash(const void *obj, Hash::Digest digest) { Silo[obj] = digest; }

  const Hash::Digest *getHash(const void *obj) {
    if (Silo.find(obj) != Silo.end()) {
      return &Silo[obj];
    }
    return nullptr;
  }

  Hash *pushHash() {
    HashStack.push_back(Hash());

    //	llvm::errs() << "  PushHash mit Groesse: " << HashStack.size() << " und
    //Rueckgabewert: " << (&HashStack.back()) << "\n";

    return &HashStack.back();
  }

  Hash::Digest popHash(const Hash *should_be = nullptr) {

    // llvm::errs() << "  PopHash mit Groesse: " << HashStack.size() << " und
    // Parameter: " << (should_be) << "\n";
    // if(should_be != &HashStack.back()){
    // llvm::errs() << "	but Stack-Level is: " << &HashStack.back() <<
    // "\n";
    //}

    assert(!should_be || should_be == &HashStack.back());

    // Finalize the Hash
    Hash::Digest digest = topHash().getDigest();
    HashStack.pop_back();
    return digest;
  }

  Hash &topHash() { return HashStack.back(); }
};

#endif
