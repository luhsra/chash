#ifndef __CHASH_VISITOR
#define __CHASH_VISITOR

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/DataCollection.h"
#include "llvm/Support/MD5.h"



#include <string>
#include <map>

#include "Hash.h"

namespace clang {

    namespace CHashConstants {
        enum  {
            Attr                           = 0x56b6cba9,
            InheritableAttr                = 0x7c0b04ce,
            InheritableParamAttr           = 0x6a4fdb90,

            NamedDecl                      = 0,
            TypeDecl                       = 0,
            Decl                           = 0,
            TagDecl                        = 0,
            VarDecl                        = 0xb19c2ee2,
            ImplicitParamDecl              = 0xd04f138f,
            ParmVarDecl                    = 0x1fe2fcb9,
            TypedefNameDecl                = 0xe8cca403,
            BlockDecl                      = 0x761e230f,
            FunctionDecl                   = 0x2a34b689,
            LabelDecl                      = 0xff6db781,
            EnumDecl                       = 0xc564aed1,
            EnumConstantDecl               = 0x11050d85,
            IndirectFieldDecl              = 0x937408ea,
            ValueDecl                      = 0xbb06d011,
            FileScopeAsmDecl               = 0x381879fa,
            CapturedDecl                   = 0xa3a884ed,
            FieldDecl                      = 0xac0c83d4,
            RecordDecl                     = 0x27892cea,

            Expr                           = 0,
            StmtExpr                       = 0xf4bb377e,
            CastExpr                       = 0x7c505e88,
            DeclRefExpr                    = 0xa33a24f3,
            PredefinedExpr                 = 0xffb3cc20,
            InitListExpr                   = 0xe23aaddd,
            UnaryExprOrTypeTraitExpr       = 0xb4995380,
            MemberExpr                     = 0xe682fc67,
            AddrLabelExpr                  = 0xe511b92e,
            CompoundLiteralExpr            = 0xc54ffefa,
            CallExpr                       = 0x427cc6e8,
            OffsetOfExpr                   = 0x48232f36,
            ParenExpr                      = 0xf1a9c911,
            AtomicExpr                     = 0x7e5497b7,
            ParenListExpr                  = 0x64600f,
            DesignatedInitExpr             = 0x8d017154,
            ArraySubscriptExpr             = 0x8c7ab6b2,
            ImplicitValueInitExpr          = 0xfe7647fa,
            VAArgExpr                      = 0xdf10fedc,
            BlockExpr                      = 0xcc75aacd,
            ShuffleVectorExpr              = 0x2e2321ad,
            ConvertVectorExpr              = 0xfe447195,
            TypeTraitExpr                  = 0xe9bda7a,
            ArrayTypeTraitExpr             = 0xd6b02f4,
            CXXBoolLiteralExpr             = 0x45b2a746,
            CXXDeleteExpr                  = 0xbcfa92ec,
            CXXFoldExpr                    = 0x1cc7935,
            ObjCPropertyRefExpr            = 0x6636c2c,
            ObjCIndirectCopyRestoreExpr    = 0xb53e833,
            ObjCBridgedCastExpr            = 0xcc79223,
            LambdaExpr                     = 0xd799f74,
            GenericSelectionExpr           = 0x51b395c,
            ExpressionTraitExpr            = 0x8f308a7,
            CharacterLiteral               = 0x2a1c033f,
            IntegerLiteral                 = 0x7b2daa87,
            FloatingLiteral                = 0xceee8473,
            StringLiteral                  = 0xe5846c45,
            ImaginaryLiteral               = 0xe340180e,

            UnaryOperator                  = 0x496a1fb5,
            BinaryOperator                 = 0xa6339d46,
            CompoundAssignmentOperator     = 0x9c582bf3,
            AbstractConditionalOperator    = 0x151982b7,
            BinaryConditionalOperator      = 0x40d2aa93,

            Stmt                           = 0,
            ForStmt                        = 0xec4e334f,
            IfStmt                         = 0x3de06c3c,
            NullStmt                       = 0x777400e0,
            DoStmt                         = 0xa80405bd,
            GotoStmt                       = 0xec2a6be8,
            ContinueStmt                   = 0x2c518360,
            ReturnStmt                     = 0x1cf8354e,
            WhileStmt                      = 0x6cb85f96,
            LabelStmt                      = 0xe3d17613,
            SwitchStmt                     = 0x6ef423db,
            CaseStmt                       = 0x9640cc21,
            DefaultStmt                    = 0x2f6febe9,
            DeclStmt                       = 0xbe748556,
            CompoundStmt                   = 0x906b6fb4,
            BreakStmt                      = 0x530ae0a9,
            GCCAsmStmt                     = 0x652782d6,
            MSAsmStmt                      = 0xccd123ef,
            AttributedStmt                 = 0x8e36d148,
            CaptureStmt                    = 0x1cafe3db,
            IndirectGotoStmt               = 0x98888356,
            AsmStmt                        = 0xe8cca40,
            CXXCatchStmt                   = 0xc853e2ac,
            ObjCAtCatchStmt                = 0xd6ce349,
            MSDependentExistsStmt          = 0xf2097b9,

            Type                           = 0xf13daabe,
            PointerType                    = 0x5b868718,
            ArrayType                      = 0xd0b37bef,
            ConstantArrayType              = 0x6439c9ef,
            VariableArrayType              = 0x74887cd4,
            ComplexType                    = 0x75d5304a,
            AtomicType                     = 0x8a024d89,
            TypeOfExprType                 = 0x3417cfda,
            TypeOfType                     = 0x98090139,
            ParenType                      = 0x7c2df2fc,
            FunctionType                   = 0x8647819b,
            FunctionProtoType              = 0x4dd5f204,
            EnumType                       = 0x4acd4cde,
            TagType                        = 0x94c7a399,
            AttributedType                 = 0xddc8426,
            UnaryTransformType             = 0xca8afa5b,
            DecayedType                    = 0x707c703e,
            AdjustedType                   = 0x9936193,
            ElaboratedType                 = 0x96681107,
            StructureType                  = 0xa5b0d36d,
            UnionType                      = 0x5057c896,
            VectorType                     = 0x4ed393c3,
            BuiltinType                    = 0xb190dc73,
            PipeType                       = 0xe9bb85af,
            RValueReferenceType            = 0xa6c4b308,
            LValueReferenceType            = 0xdb0b2b7d,
            FunctionNoProtoType            = 0x6a185f1b,
            ExtVectorType                  = 0x7d816c99,
            IncompleteArrayType            = 0xb07dce69,
            MemberPointerType              = 0x68ce4241,
            DependentAddressSpaceType      = 0x4bfa546,
            BlockPointerType               = 0x6ab898b3,
            DependentSizedExtVectorType    = 0xb5eb2dc1,
            DependentSizedArrayType        = 0xfbbc5e13,
            DecltypeType                   = 0xe61ede42,
            AutoType                       = 0x15937adc,
            DependentSizedExtVector        = 0x80161e92,
        };
    }

    template<typename H=llvm::MD5, typename HR=llvm::MD5::MD5Result>
    class CHashVisitor
        : public clang::RecursiveASTVisitor<CHashVisitor<H, HR>> {

        typedef clang::RecursiveASTVisitor<CHashVisitor<H, HR>> Inherited;
    public:
        typedef H Hash;
        typedef HR HashResult;

        /// Configure the RecursiveASTVisitor
        bool shouldWalkTypesOfTypeLocs() const { return false; }

    protected:
        ASTContext &Context;

        // For the DataCollector, we implement a few addData() functions
        void addData(uint64_t data) { topHash().update(data); }
        void addData(const StringRef &str) { topHash().update(str);}
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
    public:

#define DEF_ADD_DATA(CLASS, CODE)                   \
        template<class=void>                        \
        bool Visit##CLASS(const CLASS *S) {         \
            unsigned tag = CHashConstants::CLASS;   \
            if (tag != 0) {                         \
                addData(tag);                       \
            }                                       \
            CODE;                                   \
            return true;                            \
        }
#include "StmtDataCollectors.inc"


        CHashVisitor(ASTContext &Context) : Context(Context) { }

        /* For some special nodes, override the traverse function, since we
           need both pre- and post order traversal */
        bool TraverseTranslationUnitDecl(TranslationUnitDecl *TU) {
            if (!TU) return true;
            // First, we push a new hash onto the hashing stack. This hash
            // will capture everythin within the TU*/
            Hash *CurrentHash = pushHash();

            Inherited::WalkUpFromTranslationUnitDecl(TU);

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

        /* For some special nodes, override the traverse function, since
           we need both pre- and post order traversal. Storing of type
           hashes is done in addData() */
        bool TraverseDecl(Decl *D) {
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

            const HashResult *const SavedDigest = getHash(D);
            if (SavedDigest) {
                topHash().update(SavedDigest->Bytes);
                return true;
            }
            Hash *CurrentHash = pushHash();
            bool ret = Inherited::TraverseDecl(D);
            HashResult CurrentHashResult = popHash(CurrentHash);
            storeHash(D, CurrentHashResult);
            if (!isa<TranslationUnitDecl>(D)) {
                topHash().update(CurrentHashResult.Bytes);
            }

            return ret;
        }

        /*****************************************************************
         * When doing a semantic hash, we have to use cross-tree links to
         * other parts of the AST, here we establish these links
         */

#define DEF_TYPE_GOTO_DECL(CLASS, EXPR)         \
        bool Visit##CLASS(CLASS *T) {           \
            Inherited::Visit##CLASS(T);         \
            return TraverseDecl(EXPR);          \
        }

        DEF_TYPE_GOTO_DECL(TypedefType, T->getDecl());
        DEF_TYPE_GOTO_DECL(RecordType,  T->getDecl());
        // The EnumType forwards to the declaration. The declaration does
        // not hand back to the type.
        DEF_TYPE_GOTO_DECL(EnumType,    T->getDecl());
        bool TraverseEnumDecl(EnumDecl *E) {
            /* In the original RecursiveASTVisitor
               > if (D->getTypeForDecl()) {
               >    TRY_TO(TraverseType(QualType(D->getTypeForDecl(), 0)));
               > }
               => NO, NO, NO, to avoid endless recursion
            */
            return Inherited::WalkUpFromEnumDecl(E);
        }

        bool VisitDeclRefExpr(DeclRefExpr *E) {
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

        /*****************************************************************
         * For performance reasons, we cache some of the hashes for types
         * and declarations.
         */
        // We store hashes for declarations and types in separate maps.
    public:
        std::map<const Type *, HashResult> TypeSilo;
        std::map<const Decl *, HashResult> DeclSilo;

        void storeHash(const Type *Obj, HashResult Dig) {
            TypeSilo[Obj] = Dig;
        }

        void storeHash(const Decl *Obj, HashResult Dig) {
            DeclSilo[Obj] = Dig;
        }

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

        /*****************************************************************
         * In order to produce hashes for subtrees on the way, a hash
         * stack is used. When a new subhash is meant to be calculated, we
         * push a new stack on the hash. All hashing functions use always
         * the top of the hashing stack.
         */
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

}
#endif
