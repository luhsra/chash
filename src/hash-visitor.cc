#undef NDEBUG

#include "hash-visitor.h"

#define MAGIC_NO 1

using namespace llvm;
using namespace clang;
using namespace std;

typedef TranslationUnitHashVisitor HashVisitor;

string HashVisitor::GetHash(unsigned *processed_bytes) {
    auto hash = toplevel_hash.getDigest();
    if (processed_bytes) {
        *processed_bytes = hash.length;
    }
    return hash.asString();
}

/// Declarations

//Only called, if Decl should be visited
void HashVisitor::hashDecl(const Decl *D) {
	if (!D) {
		return;
	}
	const Hash::digest * saved_digest = GetHash(D);
	if(saved_digest){
		TopHash() << *saved_digest;
		return;
	}


	// Visit in Pre-Order
	unsigned Depth = beforeDescent();

	const Hash *hash = PushHash();

	bool handled = mt_declvisitor::Visit(D);
	if (!handled) {
		errs() << "---- START unhandled -----\n";
		D->dump();
		errs() << "---- END unhandled -----\n";

	}

	// Decls within functions are visited by the body.
	if (!isa<FunctionDecl>(*D) && hasNodes(dyn_cast<DeclContext>(D)))
		hashDeclContext(cast<DeclContext>(D));

	// visit Attributes of the Decl
	for(Attr *attr: D->attrs()){
		hashAttr(attr);
	}

	afterDescent(Depth);

	const Hash::digest digest = PopHash(hash);

	// Do not store or hash if flag is set
	if(doNotHashThis){
		doNotHashThis = false;
		return;
	}

	// Store hash for underlying type
	StoreHash(D, digest);

	// Hash into Parent
	if (!isa<TranslationUnitDecl>(D)) {
            TopHash() << digest;
	}
}

bool HashVisitor::hasNodes(const DeclContext *DC) {
	if (!DC) return false;

	return DC->hasExternalLexicalStorage() ||
		DC->noload_decls_begin() != DC->noload_decls_end();
}

void HashVisitor::hashDeclContext(const DeclContext *DC) {
	if (!DC) return;

	for (auto *D : DC->noload_decls()){
		// We don't need typedefs, Enums and Records here
		// TODO: Do we need to exclude more?
		if(!(isa<TagDecl>(D) || isa<TypedefDecl>(D))){
			hashDecl(D);
		}
	}
}



bool HashVisitor::VisitTranslationUnitDecl(const TranslationUnitDecl *Unit) {
	Hash *hash = PushHash();

	// FIXME Hash Compiler Options

	afterChildren([=] {
                StoreHash(Unit, PopHash(hash));
                toplevel_hash << *hash;
		});

	Unit->dump();

	return true;
}

bool HashVisitor::VisitVarDecl(const VarDecl *Decl) {
	//Ignore extern declarations
	if(Decl->hasExternalStorage()){
		doNotHashThis = true;
		return true;
	}
	haveSeen(Decl, Decl);

	TopHash() << AstElementVarDecl;
	hashName(Decl);
	hashType(Decl->getType());

	TopHash() << Decl->getStorageClass();
	TopHash() << Decl->getTLSKind();
	TopHash() << Decl->isModulePrivate();
	TopHash() << Decl->isNRVOVariable();

	if(Decl->hasInit()){
		const Expr *expr = Decl->getInit();
		TopHash() << AstElementVarDecl_init;
		hashStmt(expr);
	}

	return true;
}

bool HashVisitor::VisitImplicitParamDecl(const ImplicitParamDecl *Node){
	TopHash() << AstElementImplicitParamDecl;
	VisitVarDecl(Node);//important stuff done by parent

	return true;
}


bool HashVisitor::VisitParmVarDecl(const ParmVarDecl *Node){
	TopHash() << AstElementParamVarDecl;
	if(Node->hasDefaultArg()){
		hashStmt(Node->getDefaultArg());
	}
	hashType(Node->getOriginalType());
	TopHash() << Node->isParameterPack();
	VisitVarDecl(Node);//visit Parent
	return true;
}




// Types
void HashVisitor::hashType(QualType T) {
	uint64_t qualifiers = 0;
	if(T.hasQualifiers()){
		//TODO evtl. CVRMASK benutzen
		if(T.isLocalConstQualified()){
			qualifiers |= 1;
		}
		if(T.isLocalRestrictQualified()){
			qualifiers |= (1 << 1);
		}
		if(T.isLocalVolatileQualified()){
			qualifiers |= (1 << 2);
		}
		//weitere qualifier?
	}

	const Type *type = T.getTypePtr();
	assert (type != nullptr);

	if(qualifiers) {
		TopHash() << qualifiers;
	}
	//T->dump();
	//errs() << type<< " " << qualifiers << " qualifiers\n";


	const Hash::digest * saved_digest = GetHash(type);


	if(saved_digest){
		TopHash() << *saved_digest;
		return;
	}

	if(type->isStructureType()){
		if(haveSeen(type, type)){
			TopHash() << "struct";
			TopHash() << T.getAsString();
			return;
		}
	}else if(type->isUnionType()){
		if(haveSeen(type, type)){
			TopHash() << "union";
			TopHash() << T.getAsString();
			return;
		}
	}

	// Visit in Pre-Order
	unsigned Depth = beforeDescent();
	const Hash *hash = PushHash();

	bool handled = mt_typevisitor::Visit(type);
	if (!handled) {
		errs() << "---- START unhandled type -----\n";
		type->dump();
		errs() << "---- END unhandled type -----\n";

	}

	afterDescent(Depth);

	const Hash::digest digest = PopHash(hash);

	//if(saved_digest && digest != *saved_digest){
	//	errs() << "Different hashes for\n";
	//	T->dump();
	//	errs() << "\t(saved hash for)\n";
	//	type->dump();
	//}

	assert((!saved_digest || digest == *saved_digest) && "Hashes do not match");
	// Hash into Parent
	TopHash() << digest;

	// Store hash for underlying type
	StoreHash(type, digest);
}

bool HashVisitor::VisitBuiltinType(const BuiltinType *T) {
	TopHash() << T->getKind();
	assert (!T->isSugared());
	return true;
}

bool HashVisitor::VisitPointerType(const PointerType *T) {
	TopHash() << "pointer";
	hashType(T->getPointeeType());
	return true;
}

bool HashVisitor::VisitArrayType(const ArrayType *T){
	TopHash() << "ArrayType";
	hashType(T->getElementType());
	TopHash() << "[" << "*" << "]";
	return true;
}

bool HashVisitor::VisitConstantArrayType(const ConstantArrayType *T){
	TopHash() << "ConstantArrayType";
	hashType(T->getElementType());
	TopHash() << "[" << T->getSize().getZExtValue() << "]";
	return true;
}

bool HashVisitor::VisitVariableArrayType(const VariableArrayType *T){
	TopHash() << "VariableArrayType";
	hashType(T->getElementType());
	TopHash() << "[";
	hashStmt(T->getSizeExpr());
	TopHash() << "]";
	return true;
}

bool HashVisitor::VisitTypedefType(const TypedefType *T){
	return mt_typevisitor::Visit(T->desugar().getTypePtr());
}

bool HashVisitor::VisitComplexType(const ComplexType *T){
	TopHash() << "complex";
	hashType(T->getElementType());
	return true;
}

bool HashVisitor::VisitAtomicType(const AtomicType *T){
	TopHash() << "atomic";
	hashType(T->getValueType());
	return true;
}

bool HashVisitor::VisitTypeOfExprType(const TypeOfExprType *T){
	TopHash() << "typeof";
	hashType(T->desugar());
	return true;
}

bool HashVisitor::VisitTypeOfType(const TypeOfType *T){
	TopHash() << "typeoftypetype";
	hashType(T->desugar());
	return true;
}

bool HashVisitor::VisitParenType(const ParenType *T){
	TopHash() << "parenType";
	hashType(T->desugar());
	return true;
}

bool HashVisitor::VisitFunctionType(const FunctionType *T){
	TopHash() << "functype";
	hashType(T->getReturnType());
	TopHash() << T->getRegParmType();
	TopHash() << T->getCallConv();
	return true;
}

bool HashVisitor::VisitFunctionProtoType(const FunctionProtoType *T){
	TopHash() << "funcprototype";
	hashType(T->getReturnType());
	for(QualType qt: T->getParamTypes()){
		hashType(qt);
	}
	TopHash() << T->getRegParmType();
	TopHash() << T->getCallConv();
	return true;
}

bool HashVisitor::VisitEnumType(const EnumType *Node){
	TopHash() << "Enum Type";
	if(Node->isSugared()){
		hashType(Node->desugar());
	}

	EnumDecl *ed = Node->getDecl();
	hashType(ed->getIntegerType());
	hashType(ed->getPromotionType());


	for(EnumConstantDecl *ecd: ed->enumerators()){
		hashStmt(ecd->getInitExpr());
		TopHash() << ecd->getInitVal().getExtValue();
		hashName(ecd);
	}
	hashName(ed);

	return true;
}

bool HashVisitor::VisitTagType(const TagType *Node){
	TopHash() << "Tag Type";
	hashDecl(Node->getDecl());
	return true;
}

bool HashVisitor::VisitAttributedType(const AttributedType *Node){
	TopHash() << "AttributedType";
	TopHash() << Node->getAttrKind();
	hashType(Node->getModifiedType());
	hashType(Node->getEquivalentType());
	return true;
}

bool HashVisitor::VisitUnaryTransformType(const UnaryTransformType *T){
	TopHash() << "UnaryTransformType";
	hashType(T->getBaseType());
	hashType(T->getUnderlyingType());
	TopHash() << T->getUTTKind();
	return true;
}

bool HashVisitor::VisitDecayedType(const DecayedType *T){
	TopHash() << "DecayedType";
	hashType(T->getOriginalType());
	hashType(T->getAdjustedType());
	hashType(T->getPointeeType());
	return true;}

bool HashVisitor::VisitAdjustedType(const AdjustedType *T){
	TopHash() << "AdjustedType";
	hashType(T->getOriginalType());
	hashType(T->getAdjustedType());
	return true;
}

bool HashVisitor::VisitElaboratedType(const ElaboratedType *T){
	TopHash() << "ElaboratedType";
	hashType(T->getNamedType());
	TopHash() << T->getKeyword();
	return true;
}

bool HashVisitor::VisitType(const Type *T){
	const Hash::digest *digest = GetHash(T);
	if(digest){
		TopHash() << *digest;
		return true;
	}
	if(T->isStructureType()){
		haveSeen(T, T);
		TopHash() << "struct";
		const RecordType *rt = T->getAsStructureType();
		RecordDecl *rd = rt->getDecl();

		// visit Attributes of the Decl (needed because RecordDecl shouldn't be not called from hashDecl)
		for(Attr *attr: rd->attrs()){
			hashAttr(attr);
		}

		for(RecordDecl::field_iterator iter=rd->field_begin(); iter != rd->field_end(); iter++){
			FieldDecl fd = **iter;
			TopHash() << "member";
			hashType(fd.getType());
			hashName(&fd);
		}
		return true;

	}else if(T->isUnionType()){
		haveSeen(T, T);
		TopHash() << "union";
		const RecordType *rt = T->getAsUnionType();
		RecordDecl *rd = rt->getDecl();

		// visit Attributes of the Decl (needed because RecordDecl shouldn't be not called from hashDecl)
		for(Attr *attr: rd->attrs()){
			hashAttr(attr);
		}

		for(RecordDecl::field_iterator iter=rd->field_begin(); iter != rd->field_end(); iter++){
			FieldDecl fd = **iter;
			TopHash() << "member";
			hashType(fd.getType());
			hashName(&fd);
		}
		return true;

	}else{
		return false;
	}
}



// Other Utilities
void HashVisitor::hashName(const NamedDecl *ND) {
	if (ND->getIdentifier() && ND->getDeclName()) {
		TopHash() << ND->getNameAsString();
	} else {
		TopHash() << 0;
	}
}

//Expressions
bool HashVisitor::VisitCastExpr(const CastExpr *Node){
    TopHash() << AstElementCastExpr;
	hashStmt(Node->getSubExpr());
	TopHash() << Node->getCastKind();
	hashType(Node->getType());
	return true;
}

bool HashVisitor::VisitDeclRefExpr(const DeclRefExpr *Node){
	const ValueDecl *Decl = Node->getDecl();
	TopHash() << "ref";
	//FIXME:	spaeter Auskommentiertes(isa-Zeug) einkommentieren (oder Problem anders beheben)
	//		Andere Decls mehrfach referenzieren => Problem
	//		evtl. auch Zyklus in anderer Decl
	if(/*(isa<VarDecl>(Decl) || isa<FunctionDecl>(Decl)) && */ haveSeen(Decl, Decl)){
		const Hash::digest *digest = GetHash(Decl);
		if(digest){
			TopHash() << *digest;
		}else{
			//if(!isa<VarDecl>(Decl) && !isa<FunctionDecl>(Decl)){
			//	errs() << "Not a VarDecl or FunctionDecl:\n";
			//	Decl->dump();
			//}
			assert(isa<VarDecl>(Decl) || isa<FunctionDecl>(Decl));
			if(isa<VarDecl>(Decl)){
				VarDecl *vd = (VarDecl *)Decl;
				TopHash() << "VarDeclDummy";
				dummyVarDecl(vd);
			}else{
				FunctionDecl *fd = (FunctionDecl *) Decl;
				dummyFunctionDecl(fd);
			}
		}
	}else{
		hashDecl(Decl);
	}
	hashName(Node->getFoundDecl());
	return true;
}

bool HashVisitor::VisitPredefinedExpr(const PredefinedExpr *Node){
	TopHash() << "predef";
	hashType(Node->getType());
	TopHash() << Node->getFunctionName()->getString().str();
	return true;
}

bool HashVisitor::VisitCharacterLiteral(const CharacterLiteral *Node){
	TopHash() << AstElementCharacterLiteral;
	hashType(Node->getType());
	TopHash() << Node->getValue();
	return true;
}

bool HashVisitor::VisitIntegerLiteral(const IntegerLiteral *Node){
	TopHash() << AstElementIntegerLiteral;
	hashType(Node->getType());
	if(Node->getValue().isNegative()){
		TopHash() << Node->getValue().getSExtValue();
	}else{
		TopHash() << Node->getValue().getZExtValue();
	}
	return true;
}

bool HashVisitor::VisitFloatingLiteral(const FloatingLiteral *Node){
	TopHash() << AstElementFloatingLiteral;
	hashType(Node->getType());
	double mkay = (Node->getValue().convertToDouble());
	double *mmkay = &mkay;
	uint64_t *mval = (uint64_t *)(mmkay);
	uint64_t val = *mval;
	TopHash() << val;
	return true;
}

bool HashVisitor::VisitStringLiteral(const StringLiteral *Str){
	TopHash() << AstElementStringLiteral;
	hashType(Str->getType());
	TopHash() << Str->getString().str();
	return true;
}

bool HashVisitor::VisitInitListExpr(const InitListExpr *ILE){
	TopHash() << "initlistexpr";
	for(unsigned int i = 0; i < ILE->getNumInits(); i++){
		hashStmt(ILE->getInit(i));
	}
	if(ILE->hasArrayFiller()){
		hashStmt(ILE->getArrayFiller());
	}
	return true;
}

bool HashVisitor::VisitUnaryOperator(const UnaryOperator *Node){
	TopHash() << "unary";
	hashStmt(Node->getSubExpr());
	TopHash() << Node->getOpcode();
	hashType(Node->getType());
	return true;
}

bool HashVisitor::VisitUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *Node){
	TopHash() << "UOTT";
	TopHash() << Node->getKind();
	if(Node->isArgumentType()){
		hashType(Node->getArgumentType());
	}else{
		hashStmt(Node->getArgumentExpr());
	}
	return true;
}

bool HashVisitor::VisitMemberExpr(const MemberExpr *Node){
	TopHash() << "member";
	Expr *base = Node->getBase();
	hashStmt(base);

	ValueDecl *member = Node->getMemberDecl();
	hashDecl(member);

	TopHash() << Node->isArrow();

	return true;
}

bool HashVisitor::VisitBinaryOperator(const BinaryOperator *Node){
	TopHash() << "binary";
	hashStmt(Node->getLHS());
	hashStmt(Node->getRHS());
	TopHash() << Node->getOpcode();
	hashType(Node->getType());
	return true;
}

//erbt von BinaryOperator
bool HashVisitor::VisitCompoundAssignOperator(const CompoundAssignOperator *Node){
	TopHash() << "compound";
	hashStmt(Node->getLHS());
	hashStmt(Node->getRHS());
	TopHash() << Node->getOpcode();
	hashType(Node->getType());
	return true;
}

bool HashVisitor::VisitAddrLabelExpr(const AddrLabelExpr *Node){
	TopHash() << "addrlabel";
	hashStmt(Node->getLabel()->getStmt());
	return true;
}

bool HashVisitor::VisitImaginaryLiteral(const ImaginaryLiteral *Node){
	TopHash() << "imglit";
	hashType(Node->getType());
	hashStmt(Node->getSubExpr());
	return true;
}

bool HashVisitor::VisitCompoundLiteralExpr(const CompoundLiteralExpr *Node){
	TopHash() << "complit";
	hashType(Node->getType());
	hashStmt(Node->getInitializer());
	return true;
}

bool HashVisitor::VisitAbstractConditionalOperator(const AbstractConditionalOperator *Node){
	TopHash() << "ACondO";
	hashType(Node->getType());
	hashStmt(Node->getCond());
	hashStmt(Node->getTrueExpr());
	hashStmt(Node->getFalseExpr());
	return true;
}

bool HashVisitor::VisitBinaryConditionalOperator(const BinaryConditionalOperator *Node){
	TopHash() << "BCondO";
	hashType(Node->getType());
	hashStmt(Node->getCond());
	hashStmt(Node->getCommon());
	hashStmt(Node->getTrueExpr());
	hashStmt(Node->getFalseExpr());
	return true;
}

bool HashVisitor::VisitCallExpr(const CallExpr *Node){
	TopHash() << "callExpr";
	hashType(Node->getType());
	const FunctionDecl *fd = Node->getDirectCallee();
	if(fd){
		hashName(fd);
	}else{
		hashStmt(Node->getCallee());
	}
	for(unsigned int i = 0; i < Node->getNumArgs(); i++){
		hashStmt(Node->getArg(i));
	}
	return true;
}

bool HashVisitor::VisitOffsetOfExpr(const OffsetOfExpr *Node){
	TopHash() << "offsetof";
	hashType(Node->getType());
	for(unsigned int i = 0; i < Node->getNumExpressions(); i++){
		hashStmt(Node->getIndexExpr(i));
	}
	for(unsigned int i = 0; i < Node->getNumComponents(); i++){
		OffsetOfNode off = Node->getComponent(i);
		TopHash() << "offsetnode";
		TopHash() << off.getKind();
		FieldDecl *fd;

		if(off.getKind() ==  OffsetOfNode::Kind::Field){
			fd = off.getField();
			QualType t = fd->getType();
			hashType(t);

			hashName(off.getField());
		}
	}
	return true;
}

bool HashVisitor::VisitParenExpr(const ParenExpr *Node){
	TopHash() << "parenExpr";
	hashType(Node->getType());
	hashStmt(Node->getSubExpr());
	return true;
}

bool HashVisitor::VisitAtomicExpr(const AtomicExpr *Node){
	AtomicExpr *node = (AtomicExpr *)Node;

	TopHash() << "atomicExpr";
	hashType(Node->getType());
	Expr **subexprs = node->getSubExprs();
	for(unsigned int i = 0; i < node->getNumSubExprs(); i++){
		hashStmt(subexprs[i]);
	}
	return true;
}

bool HashVisitor::VisitParenListExpr(const ParenListExpr *Node){
	TopHash() << "parenListExpr";
	hashType(Node->getType());
	for(Expr *expr:((ParenListExpr *)Node)->exprs()){
		hashStmt(expr);
	}
	return true;
}

bool HashVisitor::VisitDesignatedInitExpr(const DesignatedInitExpr *Node){
	DesignatedInitExpr *node = (DesignatedInitExpr *)Node;

	TopHash() << "designatedInit";
	hashType(Node->getType());
	for(unsigned int i = 0; i < node->getNumSubExprs(); i++){
		hashStmt(node->getSubExpr(i));
	}
	for(unsigned int i = 0; i < node->size(); i++){
		DesignatedInitExpr::Designator *des = node->getDesignator(i);
		TopHash() << "designator";
		hashType(des->getField()->getType());
		hashName(des->getField());
	}
	return true;
}

bool HashVisitor::VisitStmtExpr(const StmtExpr *Node){
	TopHash() << AstElementStmtExpr;
	hashType(Node->getType());
	hashStmt(Node->getSubStmt());
	return true;
}

bool HashVisitor::VisitArraySubscriptExpr(const ArraySubscriptExpr *Node){
	TopHash() << "ArrayAccess";
	hashStmt(Node->getLHS());
	hashStmt(Node->getRHS());
	hashStmt(Node->getBase());
	hashStmt(Node->getIdx());
	return true;
}

bool HashVisitor::VisitImplicitValueInitExpr(const ImplicitValueInitExpr *Node){
	TopHash() << "implicitInit";
	hashType(Node->getType());
	return true;
}

bool HashVisitor::VisitVAArgExpr(const VAArgExpr *Node){
	TopHash() << "va_stuff";
	hashType(Node->getType());
	hashStmt(Node->getSubExpr());
	TopHash() << Node->isMicrosoftABI();
	return true;
}

bool HashVisitor::VisitBlockExpr(const BlockExpr *Node){
	TopHash() << "block expr";
	hashDecl(Node->getBlockDecl());
	hashStmt(Node->getBody());

	return true;
}

//common Decls

bool HashVisitor::VisitBlockDecl(const BlockDecl *Node)
{
	TopHash() << "blockDecl";

	for(ParmVarDecl* par: Node->parameters()){
		VisitVarDecl(par);
	}

	hashStmt(Node->getBody());
	return true;
}

bool HashVisitor::VisitFunctionDecl(const FunctionDecl *Node){
	//Ignore extern declarations
	if(Node->getStorageClass() == StorageClass::SC_Extern || Node->getStorageClass() == StorageClass::SC_PrivateExtern || !Node->isThisDeclarationADefinition()){
		doNotHashThis = true;
		return true;
	}

	haveSeen(Node, Node);

	TopHash() << "FunctionDecl";
	TopHash() << Node->getNameInfo().getName().getAsString();
	hashStmt(Node->getBody());
	TopHash() << Node->isDefined();
	TopHash() << Node->isThisDeclarationADefinition();
	TopHash() << Node->isVariadic();
	TopHash() << Node->isVirtualAsWritten();
	TopHash() << Node->isPure();
	TopHash() << Node->hasImplicitReturnZero();
	TopHash() << Node->hasPrototype();
	TopHash() << Node->hasWrittenPrototype();
	TopHash() << Node->hasInheritedPrototype();
	TopHash() << Node->isMain();
	TopHash() << Node->isExternC();
	TopHash() << Node->isGlobal();
	TopHash() << Node->isNoReturn();
	TopHash() << Node->hasSkippedBody();//???
	TopHash() << Node->getBuiltinID();

	TopHash() << Node->getStorageClass();//static and stuff
	TopHash() << Node->isInlineSpecified();
	TopHash() << Node->isInlined();



	//hash all parameters
	for(ParmVarDecl *decl: Node->parameters()){
		hashDecl(decl);
	}

	//vielleicht will man das ja auch:
	for(NamedDecl *decl: Node->getDeclsInPrototypeScope()){
		hashDecl(decl);
	}

	//visit QualType
	hashType(Node->getReturnType());

	//here an error (propably nullptr) occured
	const IdentifierInfo *ident = Node->getLiteralIdentifier();
	if(ident != nullptr){
		const char *str = ident->getNameStart();
		if(str != nullptr)
			TopHash() << str;
	}


	TopHash() << Node->getMemoryFunctionKind();//maybe needed


	return true;
}

bool HashVisitor::VisitLabelDecl(const LabelDecl *Node){
	TopHash() << "labeldecl";
	hashName(Node);
	//if location changes, then it will be recompiled there.
	//Additionally the linker has this information--> no need to handle
	//this here (SourceRange)

	TopHash() << Node->isGnuLocal();
	TopHash() << Node->isMSAsmLabel();
	if (Node->isMSAsmLabel()) {
		TopHash() << Node->getMSAsmLabel().str();
	}
	return true;
}

bool HashVisitor::VisitEnumDecl(const EnumDecl *Node){
	const Hash *hash = PushHash();
	TopHash() << "EnumDecl";
	hashName(Node);
	bool handled = true;
	for(EnumConstantDecl *ecd: Node->enumerators()){
		handled &= mt_declvisitor::Visit(ecd);
		StoreHash(ecd, TopHash().getDigest());
	}
	PopHash(hash);
	return handled;
}
bool HashVisitor::VisitEnumConstantDecl(const EnumConstantDecl *Node){
	TopHash() << "EnumConstant";
	const Expr *expr = Node->getInitExpr();
	hashName(Node);
	if(expr){
		hashStmt(expr);
	}
	TopHash() << Node->getInitVal().getExtValue();

	return true;
}



//An instance of this class is created to represent a field injected from
//an anonymous union/struct into the parent scope
//--> do not follow the struct because it does not exist then...
bool HashVisitor::VisitIndirectFieldDecl(const IndirectFieldDecl *Node){
	TopHash() << "VisitIndirectFieldDecl";
	for(IndirectFieldDecl::chain_iterator iter = Node->chain_begin();iter != Node->chain_end();iter++){
	   NamedDecl nd = **iter;
	   hashDecl(&nd);
	}
	VisitValueDecl(Node);
	return true;
}

//called by children
bool HashVisitor::VisitValueDecl(const ValueDecl *Node){

	TopHash() << "VisitValueDecl";
	hashType(Node->getType());
	hashName(Node);
	return true;
}


bool HashVisitor::VisitFileScopeAsmDecl(const FileScopeAsmDecl *Node){
	TopHash() << "FileScopeAsmDecl";

	const StringLiteral *sl = Node->getAsmString();
	if(sl != nullptr){
		hashStmt(sl);
	}
	return true;

}

bool HashVisitor::VisitCapturedDecl(const CapturedDecl *Node){
	TopHash() << "CapturedDecl";
	Stmt *body = Node->getBody();
	if(body != nullptr){
		hashStmt(body);
	}
	TopHash() << Node->isNothrow();

	for(unsigned i = 0; i < Node->getNumParams();i++){
		ImplicitParamDecl *ipd = Node->getParam(i);
		if(ipd == nullptr){
			errs() << "nullptr in CapturedDecl!";
			exit(1);
		}
		hashDecl(ipd);
	}

	return true;
}

//similar to hashDecl...
void HashVisitor::hashAttr(const Attr *attr){

	if (!attr) {
		return;
	}

	//No visitor exists. do it per hand
	//TODO ParmeterABIAttr & StmtAttr (siehe uncommented Attr...)
	if(isa<InheritableParamAttr>(attr)){
		VisitInheritableParamAttr(	(InheritableParamAttr*) attr);
	}
	else if(isa<InheritableAttr>(attr)){
		VisitInheritableAttr(  (InheritableAttr*) attr);
	}
	else{
		VisitAttr(attr);
	}
}





//Attrs
//uncommented Attr not found in namespace
bool HashVisitor::VisitAttr(const Attr *attr){
	TopHash() << "Attr";
	TopHash() << attr->getKind();//hash enum
	TopHash() << attr->isPackExpansion();


	return true;
}

bool HashVisitor::VisitInheritableAttr(const InheritableAttr *attr){
	TopHash() << "Inheritable Attr";
	VisitAttr(attr);

	return true;
}
/*
bool HashVisitor::VisitStmtAttr(const StmtAttr *attr){
	TopHash() << "Stmt Attr";
	VisitAttr(attr);

	return true;
}
*/

bool HashVisitor::VisitInheritableParamAttr(const InheritableParamAttr *attr){

	TopHash() << "InheritableParamAttr";
	VisitAttr(attr);
	return true;
}

/*
bool HashVisitor::VisitParameterABIAttr(const ParameterABIAttr *attr){

	TopHash() << "ParameterABAttr";
	const ParameterABI pabi = getABI();
	TopHash() << pabi;//struct
	VisitAttr(attr);

	return true;
}
*/



//statements
void HashVisitor::hashStmt(const Stmt *stmt){
	//Falls wir Nullpointer reinstecken
	if(!stmt){
		return;
	}

	bool handled = mt_stmtvisitor::Visit(stmt);
	if(!handled){
		errs() << "---- START unhandled statement ----\n";
		stmt->dump();
		errs() << "----- END unhandled statement -----\n";
	}
}

bool HashVisitor::VisitStmt(const Stmt *Node)
{
	errs() << "Statement Not Handled\n";
	return false;
}

bool HashVisitor::VisitCompoundStmt(const CompoundStmt *stmt){
	TopHash() << "compound";
	for(CompoundStmt::const_body_iterator iter = stmt->body_begin(); iter != stmt->body_end(); iter++){
		hashStmt(*iter);
	}
	return true;
}

bool HashVisitor::VisitBreakStmt(const BreakStmt *stmt){
	TopHash() << "break";
	return true;
}

bool HashVisitor::VisitContinueStmt(const ContinueStmt *stmt){
	TopHash() << AstElementContinueStmt;
	return true;
}

bool HashVisitor::VisitGotoStmt(const GotoStmt *stmt){
	TopHash() << AstElementGotoStmt;
	hashDecl(stmt->getLabel());
	return true;
}

bool HashVisitor::VisitLabelStmt(const LabelStmt *stmt){
	TopHash() << AstElementLabelStmt;
	hashDecl(stmt->getDecl());
	hashStmt(stmt->getSubStmt());
	return true;
}

bool HashVisitor::VisitDoStmt(const DoStmt *stmt){
	TopHash() << AstElementDoStmt;
	hashStmt(stmt->getCond());
	hashStmt(stmt->getBody());
	return true;
}

bool HashVisitor::VisitForStmt(const ForStmt *stmt){
    TopHash() << AstElementForStmt;
    hashStmt(stmt->getInit());
    hashStmt(stmt->getConditionVariableDeclStmt());
    hashStmt(stmt->getCond());
    hashStmt(stmt->getInc());
    hashStmt(stmt->getBody());
    return true;
}

bool HashVisitor::VisitIfStmt(const IfStmt *stmt){
	TopHash() << AstElementIfStmt;
	hashStmt(stmt->getConditionVariableDeclStmt());
	hashStmt(stmt->getCond());
	hashStmt(stmt->getThen());
	hashStmt(stmt->getElse());
	return true;
}

bool HashVisitor::VisitNullStmt(const NullStmt *stmt){
    //macht funktional keinen Unterschied...
    TopHash() << AstElementNullStmt;
    return true;
}

bool HashVisitor::VisitReturnStmt(const ReturnStmt *stmt){
    TopHash() << AstElementReturnStmt;
    hashStmt(stmt->getRetValue());
    return true;
}

bool HashVisitor::VisitWhileStmt(const WhileStmt *stmt){
	TopHash() << AstElementWhileStmt;
	hashStmt(stmt->getConditionVariableDeclStmt());
	hashStmt(stmt->getCond());
	hashStmt(stmt->getBody());
	return true;
}

bool HashVisitor::VisitSwitchStmt(const SwitchStmt *stmt){
	TopHash() << AstElementSwitchStmt;
	hashStmt(stmt->getConditionVariableDeclStmt());
	hashStmt(stmt->getCond());
	hashStmt(stmt->getBody());
	return true;
}

bool HashVisitor::VisitCaseStmt(const CaseStmt *stmt){
	TopHash() << AstElementCaseStmt;
	hashStmt(stmt->getLHS());
	hashStmt(stmt->getRHS());
	hashStmt(stmt->getSubStmt());
	return true;
}

bool HashVisitor::VisitDefaultStmt(const DefaultStmt *stmt){
	TopHash() << AstElementDefaultStmt;
	hashStmt(stmt->getSubStmt());
	return true;
}

bool HashVisitor::VisitDeclStmt(const DeclStmt *stmt){
	TopHash() << AstElementDeclStmt;
	for(DeclStmt::const_decl_iterator it = stmt->decl_begin(); it != stmt->decl_end(); it++){
		hashDecl(*it);
	}
	return true;
}

bool HashVisitor::VisitGCCAsmStmt(const GCCAsmStmt *stmt){
	TopHash() << "gcc asm";
	TopHash() << stmt->getAsmString()->getString().str();
	return true;
}

bool HashVisitor::VisitMSAsmStmt(const MSAsmStmt *stmt){
	TopHash() << "MS asm";
	TopHash() << stmt->getAsmString().str();
	return true;
}

bool HashVisitor::VisitAttributedStmt(const AttributedStmt *stmt){
	TopHash() << "AttributedStmt";
	for(const Attr * attr: stmt->getAttrs()){
		hashAttr(attr);
	}

	hashStmt(stmt->getSubStmt());
	return true;
}

bool HashVisitor::VisitCapturedStmt(const CapturedStmt *stmt){
	TopHash() << "CaptureStmt";
	hashStmt(stmt->getCapturedStmt());
	hashDecl(stmt->getCapturedDecl());
	return true;
}

//not tested
bool HashVisitor::VisitSEHExceptStmt(const SEHExceptStmt *stmt){
	TopHash() << "__except";
	hashStmt(stmt->getFilterExpr());
	hashStmt(stmt->getBlock());
	return true;
}

//not tested
bool HashVisitor::VisitSEHFinallyStmt(const SEHFinallyStmt *stmt){
	TopHash() << "__finally";
	hashStmt(stmt->getBlock());
	return true;
}

//not tested
bool HashVisitor::VisitSEHLeaveStmt(const SEHLeaveStmt *stmt){
	TopHash() << "__leave";
	return true;
}

//not tested
bool HashVisitor::VisitSEHTryStmt(const SEHTryStmt *stmt){
	TopHash() << "__try";
	hashStmt(stmt->getTryBlock());
	hashStmt(stmt->getHandler());
	return true;
}

bool HashVisitor::VisitIndirectGotoStmt(const IndirectGotoStmt *stmt){
	TopHash() << "IndirectGotoStmt";
	hashStmt(stmt->getTarget());
	if(stmt->getConstantTarget()){
		hashDecl(stmt->getConstantTarget());
	}
	return true;
}


//OpenMP directives, not tested
bool HashVisitor::VisitOMPExecutableDirective(const OMPExecutableDirective *stmt){
	errs() << "OMPExecutableDirectives are not implemented yet.\n";
	exit(1);
}
