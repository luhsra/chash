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


#include "SHA1.h"

using namespace clang;

class TranslationUnitHashVisitor
    : public ConstDeclVisitor<TranslationUnitHashVisitor, bool>,
      public ConstStmtVisitor<TranslationUnitHashVisitor, bool>,
      public TypeVisitor<TranslationUnitHashVisitor, bool> {

    typedef ConstDeclVisitor<TranslationUnitHashVisitor, bool> mt_declvisitor;
    typedef ConstStmtVisitor<TranslationUnitHashVisitor, bool> mt_stmtvisitor;
    typedef TypeVisitor<TranslationUnitHashVisitor, bool>      mt_typevisitor;

    sha1::SHA1 toplevel_hash;

    // In this storage we save hashes for various memory objects
    std::map<const void *, sha1::digest> silo;

    // /// Pending[i] is an action to hash an entity at level i.
    bool FirstChild;
    llvm::SmallVector<std::function<void()>, 32> Pending;

    /// Hash a child of the current node.
    unsigned beforeDescent() {
        FirstChild = true;
        return Pending.size();
    }

    template<typename Fn> void afterChildren(Fn func) {
        if (FirstChild) {
            Pending.push_back(std::move(func));
        } else {
            Pending.back()();
            Pending.back() = std::move(func);
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

    llvm::SmallVector<sha1::SHA1, 32> HashStack;
public:
    // Utilities
    bool hasNodes(const DeclContext *DC);
    void hashDeclContext(const DeclContext *DC);

    void hashDecl(const Decl *);
    void hashStmt(const Stmt *);
    void hashType(QualType);

    void hashName(const NamedDecl *);


    // C Declarations
    bool VisitTranslationUnitDecl(const TranslationUnitDecl *);
    bool VisitVarDecl(const VarDecl *);
    /// Not interesting
    bool VisitTypedefDecl(const TypedefDecl *) { return true; };

	/* Wird erst in Aufrufen geprueft */	
	bool VisitRecordDecl(const RecordDecl *D){ return true; };
	bool VisitFieldDecl(const FieldDecl *D){ return true; };


    // C Types
    bool VisitBuiltinType(const BuiltinType *);
	bool VisitPointerType(const PointerType *T);
	bool VisitArrayType(const ArrayType *T);
	bool VisitConstantArrayType(const ConstantArrayType *T);
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

    std::string GetHash();

	//C Exprs (no clang-builtins, ...)
	bool VisitExpr(const Expr *Node);	//vielleicht nicht
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
	/* might not be needed: */
	bool VisitCompoundLiteralExpr(const CompoundLiteralExpr *Node);
	bool VisitImaginaryLiteral(const ImaginaryLiteral *Node);
	bool VisitAbstractConditionalOperator(const AbstractConditionalOperator *Node);
	bool VisitBinaryConditionalOperator(const BinaryConditionalOperator *Node);
	bool VisitCallExpr(const CallExpr *Node);
	bool VisitOffsetOfExpr(const OffsetOfExpr *Node);
	bool VisitParenExpr(const ParenExpr *Node);
	bool VisitAtomicExpr(const AtomicExpr *Node);
	bool VisitParenListExpr(const ParenListExpr *Node);
	bool VisitDesignatedInitExpr(const DesignatedInitExpr *Node);
	bool VisitStmtExpr(const StmtExpr *Node);
	
	//TODO: evtl. ImplicitValueInitExpr, GenericSelectionExpr, ArraySubscriptExpr
	//TODO: evtl. OpaqueValueExpr, ExtVectorElementExpr (Beschreibung klingt nach C++)

    //functions and statements
    bool VisitFunctionDecl(const FunctionDecl *D);
    bool VisitBlockDecl(const BlockDecl *Node);
    bool VisitStmt(const Stmt *Node);
    bool VisitLabelDecl(const LabelDecl *Node);
    bool VisitEnumDecl(const EnumDecl *Node);
    bool VisitEnumConstantDecl(const EnumConstantDecl *Node);

    bool VisitImplicitParamDecl(const ImplicitParamDecl *Node);
    bool VisitParmVarDecl(const ParmVarDecl *Node);
    //DeclaratorDecl done...
    bool VisitIndirectFieldDecl(const IndirectFieldDecl *Node);
    bool VisitValueDecl(const ValueDecl *Node);//maybe called by children
    bool VisitFileScopeAsmDecl(const FileScopeAsmDecl *Node);
    bool VisitCapturedDecl(const CapturedDecl *Node);
    

	//statements
	bool VisitCompoundStmt(const CompoundStmt *stmt);
	bool VisitBreakStmt(const BreakStmt *stmt);
	bool VisitContinueStmt(const ContinueStmt *stmt);
	bool VisitGotoStmt(const GotoStmt *stmt);
	bool VisitLabelStmt(const LabelStmt *stmt);
	bool VisitDoStmt(const DoStmt *stmt);
	bool VisitForStmt(const ForStmt *stmt);
	bool VisitIfStmt(const IfStmt *stmt);
	bool VisitNullStmt(const NullStmt *stmt);
	bool VisitReturnStmt(const ReturnStmt *stmt);
	bool VisitWhileStmt(const WhileStmt *stmt);
	bool VisitSwitchStmt(const SwitchStmt *stmt);
	bool VisitCaseStmt(const CaseStmt *stmt);
	bool VisitDefaultStmt(const DefaultStmt *stmt);
	bool VisitDeclStmt(const DeclStmt *stmt);
	//TODO: spaeter: AsmStmt
	//TODO: vllt. AttributedStmt, CapturedStmt, IndirectGotoStmt, SEH*Stmts

protected:
	std::map<const void *, const Type *> seen_types;
		
	bool haveSeen(const void *key, const Type *type){
		if (seen_types.find(key) != seen_types.end()){
			return true;
		}
		seen_types[key] = type;
		return false;
	}

    // Hash Silo
    void StoreHash(const void *obj, sha1::digest digest) {
        silo[obj] = digest;
    }

    const sha1::digest * GetHash(const void *obj) {
        if (silo.find(obj) != silo.end()) {
            return &silo[obj];
        }
        return nullptr;
    }

    sha1::SHA1 * PushHash() {
        HashStack.push_back(sha1::SHA1());
        return &HashStack.back();
    }

    sha1::digest PopHash(const sha1::SHA1 *should_be = nullptr) {
        assert(!should_be || should_be == &HashStack.back());

        // Finalize the Hash
        sha1::digest digest;
        HashStack.back().getDigest(digest.value);
        HashStack.pop_back();
        return digest;
    }

    sha1::SHA1 &Hash() {
        return HashStack.back();
    }

	sha1::digest getDigest(){
		sha1::digest digest;
        HashStack.back().getDigest(digest.value);
		return digest;
	}
};

#endif
