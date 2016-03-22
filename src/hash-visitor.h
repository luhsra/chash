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
	typedef TypeVisitor<TranslationUnitHashVisitor, bool>	   mt_typevisitor;

	Hash toplevel_hash;

	// In this storage we save hashes for various memory objects
	std::map<const void *, Hash::digest> silo;

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
	bool VisitRecordDecl(const RecordDecl *D){ return true; };
	bool VisitFieldDecl(const FieldDecl *D){ return true; };


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

	std::string GetHash();

	//C Exprs (no clang-builtins, ...)
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
	bool VisitAbstractConditionalOperator(const AbstractConditionalOperator *Node);
	bool VisitBinaryConditionalOperator(const BinaryConditionalOperator *Node);
	bool VisitCallExpr(const CallExpr *Node);
	bool VisitOffsetOfExpr(const OffsetOfExpr *Node);
	bool VisitParenExpr(const ParenExpr *Node);
	bool VisitAtomicExpr(const AtomicExpr *Node);
	bool VisitParenListExpr(const ParenListExpr *Node);
	bool VisitDesignatedInitExpr(const DesignatedInitExpr *Node);
	bool VisitStmtExpr(const StmtExpr *Node);
	bool VisitVAArgExpr(const VAArgExpr *Node);

	//TODO: evtl. ImplicitValueInitExpr, GenericSelectionExpr, ArraySubscriptExpr
	//TODO: evtl. OpaqueValueExpr, ExtVectorElementExpr (Beschreibung klingt nach C++)

	//declarations
	bool VisitFunctionDecl(const FunctionDecl *D);
	bool VisitBlockDecl(const BlockDecl *Node);
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

	//Attrs
	//uncommented Attr not found in namespace
	bool VisitAttr(const Attr *attr);
	bool VisitInheritableAttr(const InheritableAttr *attr);
	//bool VisitStmtAttr(const StmtAttr *attr);
	bool VisitInheritableParamAttr(const InheritableParamAttr *attr);
	//bool VisitParameterABIAttr(const ParameterABIAttr *attr);

	//statements
	bool VisitStmt(const Stmt *Node);
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
	bool VisitGCCAsmStmt(const GCCAsmStmt *stmt);
	bool VisitMSAsmStmt(const MSAsmStmt *stmt);

	//not sure if we need this
	bool VisitAttributedStmt(const AttributedStmt *stmt);
	bool VisitCapturedStmt(const CapturedStmt *stmt);
	bool VisitSEHExceptStmt(const SEHExceptStmt *stmt);
	bool VisitSEHFinallyStmt(const SEHFinallyStmt *stmt);
	bool VisitSEHLeaveStmt(const SEHLeaveStmt *stmt);
	bool VisitSEHTryStmt(const SEHTryStmt *stmt);
	bool VisitIndirectGotoStmt(const IndirectGotoStmt *stmt);

	//not implemented
	bool VisitOMPExecutableDirective(const OMPExecutableDirective *stmt);

protected:
	bool doNotHashThis = false; // Flag used to ignore Nodes such as extern Decls
	std::map<const void *, const void *> seen_types;

	bool haveSeen(const void *key, const void *type){
		if (seen_types.find(key) != seen_types.end()){
			return true;
		}
		seen_types[key] = type;
		return false;
	}

	bool dummyFunctionDecl(FunctionDecl *fd){
		//Ignore extern declarations
		if(fd->getStorageClass() == StorageClass::SC_Extern || fd->getStorageClass() == StorageClass::SC_PrivateExtern){
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
		Hash() << fd->hasSkippedBody();//???
		Hash() << fd->getBuiltinID();

		Hash() << fd->getStorageClass();//static and stuff
		Hash() << fd->isInlineSpecified();
		Hash() << fd->isInlined();



		//hash all parameters
		for(ParmVarDecl *decl: fd->parameters()){
			hashDecl(decl);
		}

		//vielleicht will man das ja auch:
		for(NamedDecl *decl: fd->getDeclsInPrototypeScope()){
			hashDecl(decl);
		}

		//visit QualType
		hashType(fd->getReturnType());

		//here an error (propably nullptr) occured
		const IdentifierInfo *ident = fd->getLiteralIdentifier();
		if(ident != nullptr){
			const char *str = ident->getNameStart();
			if(str != nullptr)
				Hash() << str;
		}


		Hash() << fd->getMemoryFunctionKind();//maybe needed
		return true;
	}

	void dummyVarDecl(VarDecl *vd){
		hashName(vd);
		hashType(vd->getType());
		Hash() << vd->getStorageClass();
		Hash() << vd->getTLSKind();
		Hash() << vd->isModulePrivate();
		Hash() << vd->isNRVOVariable();
	}

	// Hash Silo
	void StoreHash(const void *obj, Hash::digest digest) {
		silo[obj] = digest;
	}

	const Hash::digest * GetHash(const void *obj) {
		if (silo.find(obj) != silo.end()) {
			return &silo[obj];
		}
		return nullptr;
	}

	Hash * PushHash() {
		HashStack.push_back(Hash());

                //	llvm::errs() << "  PushHash mit Groesse: " << HashStack.size() << " und Rueckgabewert: " << (&HashStack.back()) << "\n";

		return &HashStack.back();
	}

	Hash::digest PopHash(const Hash *should_be = nullptr) {

            // llvm::errs() << "  PopHash mit Groesse: " << HashStack.size() << " und Parameter: " << (should_be) << "\n";
            //if(should_be != &HashStack.back()){
            //llvm::errs() << "	but Stack-Level is: " << &HashStack.back() << "\n";
            //}

            assert(!should_be || should_be == &HashStack.back());

            // Finalize the Hash
            Hash::digest digest = TopHash().getDigest();
            HashStack.pop_back();
            return digest;
	}

	Hash &TopHash() {
		return HashStack.back();
	}
};

#endif
