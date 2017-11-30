#undef NDEBUG

#include "hash-visitor.h"

using namespace llvm;
using namespace clang;

bool CHashVisitor::TraverseTranslationUnitDecl(TranslationUnitDecl *TU) {
    if (!TU) return true;
    // First, we push a new hash onto the hashing stack. This hash
    // will capture everythin within the TU*/
    Hash *CurrentHash = pushHash();

    WalkUpFromTranslationUnitDecl(TU);

    // Do recursion on our own, since we want to exclude some children
    const auto DC = cast<DeclContext>(TU);
    for (auto * Child : DC->noload_decls()) {
        if (   isa<TypedefDecl>(Child)
            || isa<RecordDecl>(Child)
            || isa<EnumDecl>(Child))
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
            if (!doHashing) continue;
          }
        }

        TraverseDecl(Child);
    }

    storeHash(TU, popHash(CurrentHash));

    return true;
}

bool CHashVisitor::TraverseDecl(Decl *D) {
    if (!D) return true;
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

    const Hash::Digest *const SavedDigest = getHash(D);
    if (SavedDigest) {
        topHash() << *SavedDigest;
        return true;
    }
    Hash *CurrentHash = pushHash();
    bool ret = Inherited::TraverseDecl(D);
    storeHash(D, popHash(CurrentHash));
    if (!isa<TranslationUnitDecl>(D)) {
        topHash() << *CurrentHash;
    }

    return ret;
}

bool CHashVisitor::VisitDeclRefExpr(DeclRefExpr *E) {
        ValueDecl * ValDecl = E->getDecl();
        // Function Declarations are handled in VisitCallExpr
        if (!ValDecl) { return true; }
        if (isa<VarDecl>(ValDecl)) {
            /* We emulate TraverseDecl here for VarDecl, because we
             * are not allowed to call TraverseDecl here, since the
             * initial expression of a DeclRefExpr might reference a
             * sourronding Declaration itself. For example:
             *
             * struct foo {int N;}
             * struct foo a = { sizeof(a) };
             */
            VarDecl * VD = static_cast<VarDecl *>(ValDecl);
            VisitNamedDecl(VD);
            TraverseType(VD->getType());
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

// On our way down, we meet a lot of qualified types.
void CHashVisitor::addData(const QualType &T) {
    // 1. Hash referenced type
    const Type *const ActualType = T.getTypePtr();
    assert(ActualType != nullptr);

    // FIXME: Structural hash
    // 1.1 Was it already hashed?
    const Hash::Digest *const SavedDigest = getHash(ActualType);
    if (SavedDigest) {
        // 1.1.1 Use cached value
        topHash() << *SavedDigest;
    } else {
        // 1.1.2 Calculate hash for type
        const Hash *const CurrentHash = pushHash();
        TraverseType(T); // Uses getTypePtr() internally
        const Hash::Digest TypeDigest = popHash(CurrentHash);
        topHash() << TypeDigest;

        // Store hash for underlying type
        storeHash(ActualType, TypeDigest);
    }

    // Add the qulaifiers at this specific usage of the type
    topHash() << T.getCVRQualifiers();
}
