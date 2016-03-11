#undef NDEBUG

#include "hash-visitor.h"

#define MAGIC_NO 1

using namespace llvm;
using namespace clang;
using namespace std;
using namespace sha1;

typedef TranslationUnitHashVisitor HashVisitor;

string HashVisitor::GetHash() {
    return toplevel_hash.getHexDigest();
}

/// Declarations

//Only called, if Decl should be visited
void HashVisitor::hashDecl(const Decl *D) {
    if (!D) {
        return;
    }
    //errs() << "hashDECL\n";
    const sha1::digest * saved_digest = GetHash(D);
    //errs() << "GETHASH ";
    //D->dump();
    if(saved_digest){
        //errs() << saved_digest;
        //D->dump();
        //errs() << *saved_digest;
        //errs() << "HASHSTACKSIZE " << HashStack.size() << "\n";
        Hash() << *saved_digest;
        //errs() << "GOOD\n";
        return;
    }
    

    // Visit in Pre-Order
    unsigned Depth = beforeDescent();

    const sha1::SHA1 *hash = PushHash();

    bool handled = mt_declvisitor::Visit(D);
    if (!handled) {
        errs() << "---- START unhandled -----\n";
        D->dump();
        errs() << "---- END unhandled -----\n";

    }

    // Decls within functions are visited by the body.
    if (!isa<FunctionDecl>(*D) && hasNodes(dyn_cast<DeclContext>(D)))
        hashDeclContext(cast<DeclContext>(D));

    afterDescent(Depth);

    const sha1::digest digest = PopHash(hash);

    // Store hash for underlying type
    StoreHash(D, digest);

    // Hash into Parent
    if (!isa<TranslationUnitDecl>(D)) {
        Hash() << digest;
    }
}

bool HashVisitor::hasNodes(const DeclContext *DC) {
    if (!DC) return false;

    return DC->hasExternalLexicalStorage() ||
        DC->noload_decls_begin() != DC->noload_decls_end();
}

void HashVisitor::hashDeclContext(const DeclContext *DC) {
    if (!DC) return;

    for (auto *D : DC->noload_decls())
        hashDecl(D);
}



bool HashVisitor::VisitTranslationUnitDecl(const TranslationUnitDecl *Unit) {
    sha1::SHA1 *hash = PushHash();

    // errs() << "TU start " << Decl << "\n";
    // FIXME Hash Compiler Options

    afterChildren([=] {
            StoreHash(Unit, PopHash(hash));
            toplevel_hash << *hash;
            // errs() << "TU " << Decl << " " << hash->getHexDigest() << "\n";
        });

    return true;
}

bool HashVisitor::VisitVarDecl(const VarDecl *Decl) {
    Hash() << "VarDecl";
    hashName(Decl);
    hashType(Decl->getType());

    Hash() << Decl->getStorageClass();
    Hash() << Decl->getTLSKind();
    Hash() << Decl->isModulePrivate();
    Hash() << Decl->isNRVOVariable();
	bool handled = true;

	if(Decl->hasInit()){
		const Expr *expr = Decl->getInit();
		const sha1::SHA1 *hash = PushHash();
		handled &= mt_stmtvisitor::Visit(expr);
		const sha1::digest digest = PopHash(hash);
		Hash() << "init";
		Hash() << digest;
		if (!handled) {
        	errs() << "---- START unhandled Expr -----\n";
        	expr->dump();
        	errs() << "---- END unhandled Expr -----\n";
    	}
	}
    Hash() << "END VarDecl";

    return true;
}

bool HashVisitor::VisitImplicitParamDecl(const ImplicitParamDecl *Node){

    Hash() << "ImplicitParamDecl";
    VisitVarDecl(Node);//important stuff done by parent


    return true;
}


bool HashVisitor::VisitParmVarDecl(const ParmVarDecl *Node){
    Hash() << "ParmVarDecl";
    if(Node->hasDefaultArg()){
        hashStmt(Node->getDefaultArg());
    }
    hashType(Node->getOriginalType());
    Hash() << Node->isParameterPack();
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

    // Visit in Pre-Order
    unsigned Depth = beforeDescent();
    const sha1::SHA1 *hash = PushHash();

	if(qualifiers) {
		Hash() << qualifiers;
	}

    bool handled = mt_typevisitor::Visit(type);
    if (!handled) {
        errs() << "---- START unhandled type -----\n";
        type->dump();
        errs() << "---- END unhandled type -----\n";

    }

    afterDescent(Depth);

    const sha1::digest digest = PopHash(hash);

    // Hash into Parent
    Hash() << digest;
	
    // This will be the root of a future optimization
    const sha1::digest * saved_digest = GetHash(type);
    assert(!saved_digest || digest == *saved_digest && "Hashes do not match");

    // Store hash for underlying type
    StoreHash(type, digest);

    // DEBUG OUTPUT
    // type->dump();
    // errs() << digest.getHexDigest() << "\n";
}

bool HashVisitor::VisitBuiltinType(const BuiltinType *T) {
    Hash() << T->getKind();
    assert (!T->isSugared());
    return true;
}

bool HashVisitor::VisitPointerType(const PointerType *T) {
	Hash() << "pointer";
	const sha1::digest *digest = GetHash(T->getPointeeType().getTypePtr());
	if(digest){
		Hash() << *digest;
		return true;
	}
	//FIXME: evtl. FunctionPointerType (erst Testsysteme)
	if((T->getPointeeType()).getTypePtr()->isStructureType()){
		if(haveSeen(T->getPointeeType().getTypePtr(), T->getPointeeType().getTypePtr())){
			Hash() << "struct";
			Hash() << (T->getPointeeType()).getAsString();
		}else{
			//rekursiv absteigen...
			hashType(T->getPointeeType());
		}
	}else if((T->getPointeeType()).getTypePtr()->isUnionType()){
		if(haveSeen(T->getPointeeType().getTypePtr(), T->getPointeeType().getTypePtr())){
			Hash() << "union";
			Hash() << (T->getPointeeType()).getAsString();
		}else{
			//rekursiv absteigen...
			hashType(T->getPointeeType());
		}
	}else{
		hashType(T->getPointeeType());
	}
	return true;
}

bool HashVisitor::VisitArrayType(const ArrayType *T){
	Hash() << "Arraytype";
	hashType(T->getElementType());
	Hash() << "[" << "*" << "]";
	return true;
}

bool HashVisitor::VisitConstantArrayType(const ConstantArrayType *T){
	Hash() << "ArraytypeC";
	hashType(T->getElementType());
	Hash() << "[" << T->getSize().getZExtValue() << "]";
	return true;
}

bool HashVisitor::VisitVariableArrayType(const VariableArrayType *T){
	Hash() << "ArraytypeV";
	hashType(T->getElementType());
	Hash() << "[";
	hashStmt(T->getSizeExpr());
	Hash() << "]";
	return true;
}

bool HashVisitor::VisitTypedefType(const TypedefType *T){
	return mt_typevisitor::Visit(T->desugar().getTypePtr());
}

bool HashVisitor::VisitComplexType(const ComplexType *T){
	hashType(T->getElementType());
	Hash() << "complex";
	return true;
}

bool HashVisitor::VisitAtomicType(const AtomicType *T){
	hashType(T->getValueType());
	Hash() << "atomic";
	return true;
}

bool HashVisitor::VisitTypeOfExprType(const TypeOfExprType *T){
	hashType(T->desugar());
	Hash() << "typeof";
	return true;
}

bool HashVisitor::VisitTypeOfType(const TypeOfType *T){
	hashType(T->desugar());
	Hash() << "typeoftypetype";
	return true;
}

bool HashVisitor::VisitParenType(const ParenType *T){
		hashType(T->desugar());
		Hash() << "parenType";
		return true;
}

bool HashVisitor::VisitFunctionType(const FunctionType *T){
	hashType(T->getReturnType());
	Hash() << "functype";
	Hash() << T->getRegParmType();
	Hash() << T->getCallConv();
	return true;
}

bool HashVisitor::VisitFunctionProtoType(const FunctionProtoType *T){
	hashType(T->getReturnType());
	for(QualType qt: T->getParamTypes()){
		hashType(qt);
	}
	Hash() << "funcytype";
	Hash() << T->getRegParmType();
	Hash() << T->getCallConv();
	return true;
}

bool HashVisitor::VisitEnumType(const EnumType *Node){
    Hash() << "Enum Type";
    if(Node->isSugared()){
        hashType(Node->desugar());
    }

    EnumDecl *ed = Node->getDecl();
    hashType(ed->getIntegerType());
    hashType(ed->getPromotionType());


    for(EnumConstantDecl *ecd: ed->enumerators()){
        hashStmt(ecd->getInitExpr());
        Hash() << ecd->getInitVal().getExtValue();
        hashName(ecd);
    }
    hashName(ed);

    return true;
}

//TODO: Needed: ?
bool HashVisitor::VisitTagType(const TagType *Node){
    Hash() << "Tag Type";
    hashDecl(Node->getDecl());
    return true;
}

bool HashVisitor::VisitAttributedType(const AttributedType *Node){
	Hash() << "AttributedType";
	Hash() << Node->getAttrKind();
	hashType(Node->getModifiedType());
	hashType(Node->getEquivalentType());
	return true;
}

bool HashVisitor::VisitUnaryTransformType(const UnaryTransformType *T){
	Hash() << "UnaryTransformType";
	hashType(T->getBaseType());
	hashType(T->getUnderlyingType());
	Hash() << T->getUTTKind();
	return true;
}

bool HashVisitor::VisitDecayedType(const DecayedType *T){
	Hash() << "DecayedType";
	hashType(T->getOriginalType());
	hashType(T->getAdjustedType());
	hashType(T->getPointeeType());
	return true;}

bool HashVisitor::VisitAdjustedType(const AdjustedType *T){
	Hash() << "AdjustedType";
	hashType(T->getOriginalType());
	hashType(T->getAdjustedType());
	return true;	
}

bool HashVisitor::VisitType(const Type *T){
	const sha1::digest *digest = GetHash(T);
	if(digest){
		Hash() << *digest;
		return true;
	}
	if(T->isStructureType()){
		haveSeen(T, T);
		Hash() << "struct";
		const RecordType *rt = T->getAsStructureType();
		RecordDecl *rd = rt->getDecl();
		for(RecordDecl::field_iterator iter=rd->field_begin(); iter != rd->field_end(); iter++){
			FieldDecl fd = **iter;
			Hash() << "member";
			hashType(fd.getType());
			hashName(&fd);
		}
		return true;
		
	}else if(T->isUnionType()){
		haveSeen(T, T);
		Hash() << "union";
		const RecordType *rt = T->getAsUnionType();
		RecordDecl *rd = rt->getDecl();
		for(RecordDecl::field_iterator iter=rd->field_begin(); iter != rd->field_end(); iter++){
			FieldDecl fd = **iter;
			Hash() << "member";
			hashType(fd.getType());
			hashName(&fd);
		}
		return true;
		
	}else{ //FIXME: evtl. FunctionPointerType (erst Testsysteme)
		return false;
	}
}



// Other Utilities
void HashVisitor::hashName(const NamedDecl *ND) {
    if (ND->getDeclName()) {
        Hash() << ND->getNameAsString();
    } else {
        Hash() << 0;
    }
}

//Expressions
bool HashVisitor::VisitCastExpr(const CastExpr *Node){
	hashStmt(Node->getSubExpr());
	Hash() << "cast";
	Hash() << Node->getCastKind();
	hashType(Node->getType());
	return true;	
}

bool HashVisitor::VisitDeclRefExpr(const DeclRefExpr *Node){
	const ValueDecl *vd = Node->getDecl();
	Hash() << "ref";
	hashDecl(vd);
	hashName(Node->getFoundDecl());
    Hash() << "after ref";
	return true;
}

bool HashVisitor::VisitPredefinedExpr(const PredefinedExpr *Node){
	Hash() << "predef";
	hashType(Node->getType());
	Hash() << Node->getFunctionName()->getString().str();
	return true;
}

bool HashVisitor::VisitCharacterLiteral(const CharacterLiteral *Node){
	Hash() << "literal";	
	hashType(Node->getType());
	Hash() << Node->getValue();
	return true;
}

bool HashVisitor::VisitIntegerLiteral(const IntegerLiteral *Node){
	Hash() << "literal";	
	hashType(Node->getType());
	if(Node->getValue().isNegative()){
		Hash() << Node->getValue().getSExtValue();
	}else{
		Hash() << Node->getValue().getZExtValue();
	}
	return true;
}

bool HashVisitor::VisitFloatingLiteral(const FloatingLiteral *Node){
	Hash() << "literal";	
	hashType(Node->getType());
	double mkay = (Node->getValue().convertToDouble());
	double *mmkay = &mkay;
	uint64_t *mval = (uint64_t *)(mmkay);
	uint64_t val = *mval;
	Hash() << val;
	return true;
}

bool HashVisitor::VisitStringLiteral(const StringLiteral *Str){
	Hash() << "literal";	
	hashType(Str->getType());
	Hash() << Str->getString().str();
	return true;
}

bool HashVisitor::VisitInitListExpr(const InitListExpr *ILE){
	for(unsigned int i = 0; i < ILE->getNumInits(); i++){
		hashStmt(ILE->getInit(i));
	}
	Hash() << "list";
	return true;
}

bool HashVisitor::VisitUnaryOperator(const UnaryOperator *Node){
	hashStmt(Node->getSubExpr());
	Hash() << "unary";
	Hash() << Node->getOpcode();
	hashType(Node->getType());
	return true;
}

bool HashVisitor::VisitUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *Node){
	if(Node->isArgumentType()){
        Hash() << "UOTT";
		Hash() << Node->getKind();
		hashType(Node->getArgumentType());
		return true;
	}else{
		hashStmt(Node->getArgumentExpr());
		Hash() << "UOTT";
		Hash() << Node->getKind();
        return true;
	}
}

bool HashVisitor::VisitMemberExpr(const MemberExpr *Node){
    //TODO testen
    Hash() << "member";
    Expr *base = Node->getBase();
    hashStmt(base);

    //hashType(Node->);
    ValueDecl *member = Node->getMemberDecl();
    hashDecl(member);

    Hash() << Node->isArrow();



	return true;
}

bool HashVisitor::VisitBinaryOperator(const BinaryOperator *Node){
	hashStmt(Node->getLHS());
	hashStmt(Node->getRHS());
	Hash() << "binary";
	Hash() << Node->getOpcode();
	hashType(Node->getType());
	return true;
}

//erbt von BinaryOperator
bool HashVisitor::VisitCompoundAssignOperator(const CompoundAssignOperator *Node){	
	hashStmt(Node->getLHS());
	hashStmt(Node->getRHS());
	Hash() << "compound";
	Hash() << Node->getOpcode();
	hashType(Node->getType());
	return true;
}

bool HashVisitor::VisitAddrLabelExpr(const AddrLabelExpr *Node){
	hashStmt(Node->getLabel()->getStmt());
	Hash() << "addrlabel";
	return true;
}

bool HashVisitor::VisitImaginaryLiteral(const ImaginaryLiteral *Node){
	Hash() << "imglit";	
	hashType(Node->getType());
	hashStmt(Node->getSubExpr());
	return true;
}

bool HashVisitor::VisitCompoundLiteralExpr(const CompoundLiteralExpr *Node){
	Hash() << "complit";	
	hashType(Node->getType());
	hashStmt(Node->getInitializer());
	return true;
}

bool HashVisitor::VisitAbstractConditionalOperator(const AbstractConditionalOperator *Node){
	Hash() << "ACondO";	
	hashType(Node->getType());
	hashStmt(Node->getCond());
	hashStmt(Node->getTrueExpr());
	hashStmt(Node->getFalseExpr());
	return true;
}

bool HashVisitor::VisitBinaryConditionalOperator(const BinaryConditionalOperator *Node){
	Hash() << "BCondO";	
	hashType(Node->getType());
	hashStmt(Node->getCond());
	hashStmt(Node->getCommon());
	hashStmt(Node->getTrueExpr());
	hashStmt(Node->getFalseExpr());
	return true;
}

bool HashVisitor::VisitCallExpr(const CallExpr *Node){
	Hash() << "callExpr";	
	hashType(Node->getType());
	hashName(Node->getDirectCallee());
	for(Stmt *subex: ((CallExpr *)Node)->getRawSubExprs()){
		hashStmt(subex);
	}
	return true;
}

bool HashVisitor::VisitOffsetOfExpr(const OffsetOfExpr *Node){
	Hash() << "offsetof";	
	hashType(Node->getType());
	for(unsigned int i = 0; i < Node->getNumExpressions(); i++){
		hashStmt(Node->getIndexExpr(i));
	}
	for(unsigned int i = 0; i < Node->getNumComponents(); i++){
		OffsetOfNode off = Node->getComponent(i);
		Hash() << "offsetnode";
		Hash() << off.getKind();
		hashType(off.getField()->getType());
		hashName(off.getField());
	}
	return true;
}

bool HashVisitor::VisitParenExpr(const ParenExpr *Node){
	Hash() << "parenExpr";	
	hashType(Node->getType());
	hashStmt(Node->getSubExpr());
	return true;
}

bool HashVisitor::VisitAtomicExpr(const AtomicExpr *Node){
	AtomicExpr *node = (AtomicExpr *)Node;

	Hash() << "atomicExpr";	
	hashType(Node->getType());
	Expr **subexprs = node->getSubExprs();
	for(unsigned int i = 0; i < node->getNumSubExprs(); i++){
		hashStmt(subexprs[i]);
	}
	return true;
}

bool HashVisitor::VisitParenListExpr(const ParenListExpr *Node){
	Hash() << "parenListExpr";	
	hashType(Node->getType());
	for(Expr *expr:((ParenListExpr *)Node)->exprs()){
		hashStmt(expr);
	}
	return true;
}

bool HashVisitor::VisitDesignatedInitExpr(const DesignatedInitExpr *Node){
	DesignatedInitExpr *node = (DesignatedInitExpr *)Node;

	Hash() << "designatedInit";	
	hashType(Node->getType());
	for(unsigned int i = 0; i < node->getNumSubExprs(); i++){
		hashStmt(node->getSubExpr(i));
	}
	for(unsigned int i = 0; i < node->size(); i++){
		DesignatedInitExpr::Designator *des = node->getDesignator(i);
		Hash() << "designator";
		hashType(des->getField()->getType());
		hashName(des->getField());
	}
	return true;
}

bool HashVisitor::VisitStmtExpr(const StmtExpr *Node){
	Hash() << "stmtExpr";	
	hashType(Node->getType());
	hashStmt(Node->getSubStmt());
	return true;
}

bool HashVisitor::VisitBlockExpr(const BlockExpr *Node){
    Hash() << "block expr";
    hashDecl(Node->getBlockDecl());
    hashStmt(Node->getBody());

    return true;
}

//common Decls

bool HashVisitor::VisitBlockDecl(const BlockDecl *Node)
{
    //TODO
    Hash() << "blockDecl";

/*
    for(BlockDecl::param_iterator iterator = Node->param_begin(); iterator != Node->param_end();iterator++){
        ParmVarDecl element = **iterator;
        hashType(element.getOriginalType());
        hashName(&element);
    }
 */
    for(ParmVarDecl* par: Node->parameters()){
        VisitVarDecl(par);
    }

    return mt_stmtvisitor::Visit(Node->getBody());

}

bool HashVisitor::VisitFunctionDecl(const FunctionDecl *Node){

    bool handled = true;

    Hash() << "FunctionDecl";
    Hash() << Node->getNameInfo().getName().getAsString();
    //Hash() << Node->containsUnexpandedParameterPack();
    if(Node->hasBody()){
        handled &= mt_stmtvisitor::Visit(Node->getBody());
    }
    Hash() << Node->isDefined();
    Hash() << Node->isThisDeclarationADefinition();
    Hash() << Node->isVariadic();
    Hash() << Node->isVirtualAsWritten();
    Hash() << Node->isPure();
    Hash() << Node->hasImplicitReturnZero();
    Hash() << Node->hasPrototype();
    Hash() << Node->hasWrittenPrototype();
    Hash() << Node->hasInheritedPrototype();
    Hash() << Node->isMain();
    Hash() << Node->isExternC();
    Hash() << Node->isGlobal();
    Hash() << Node->isNoReturn();
    Hash() << Node->hasSkippedBody();//???
    Hash() << Node->getBuiltinID();

    Hash() << Node->getStorageClass();//static and stuff
    Hash() << Node->isInlineSpecified();
    Hash() << Node->isInlined();



    //hash all parameters
    for(ParmVarDecl *decl: Node->parameters()){
        //handled &= mt_declvisitor::Visit(decl);
        hashDecl(decl);
    }

    //vielleicht will man das ja auch:
    for(NamedDecl *decl: Node->getDeclsInPrototypeScope()){
        //handled &= mt_declvisitor::Visit(decl);
        hashDecl(decl);
    }

    //visit QualType
    hashType(Node->getReturnType());

    //here an error (propably nullptr) occured
    const IdentifierInfo *ident = Node->getLiteralIdentifier();
    if(ident != nullptr){
        const char *str = ident->getNameStart();
        if(str != nullptr)
            Hash() << str;
    }


    Hash() << Node->getMemoryFunctionKind();//maybe needed


    return true;
}

bool HashVisitor::VisitLabelDecl(const LabelDecl *Node){
    Hash() << "LabelDecl";
    hashStmt(Node->getStmt());
    //if location changes, then it will be recompiled there.
    //Additionally the linker has this information--> no need to handle
    //this here (SourceRange)

    Hash() << Node->getMSAsmLabel().str();
    return true;
}

bool HashVisitor::VisitEnumDecl(const EnumDecl *Node){
    const sha1::SHA1 *hash = PushHash();
    Hash() << "EnumDecl";
    hashName(Node);
    bool handled = true;
    for(EnumConstantDecl *ecd: Node->enumerators()){
        handled &= mt_declvisitor::Visit(ecd);
        StoreHash(ecd, getDigest());
    }
    PopHash(hash);
    return handled;
}
bool HashVisitor::VisitEnumConstantDecl(const EnumConstantDecl *Node){
    Hash() << "EnumConstant";
    const Expr *expr = Node->getInitExpr();
    hashName(Node);
    if(expr){
        hashStmt(expr);
    }else{
        Hash() << "NULL in InitExpr";
    }
    Hash() << Node->getInitVal().getExtValue(); //TODO: gebraucht?
    
    return true;
}



//An instance of this class is created to represent a field injected from
//an anonymous union/struct into the parent scope
//--> do not follow the struct because it does not exist then...
bool HashVisitor::VisitIndirectFieldDecl(const IndirectFieldDecl *Node){
    Hash() << "VisitIndirectFieldDecl";
    for(IndirectFieldDecl::chain_iterator iter = Node->chain_begin();iter != Node->chain_end();iter++){
       NamedDecl nd = **iter;
       hashDecl(&nd);
    }
    VisitValueDecl(Node);
    return true;
}

//called by children
bool HashVisitor::VisitValueDecl(const ValueDecl *Node){

    Hash() << "VisitValueDecl";
    hashType(Node->getType());
    hashName(Node);
    return true;
}


bool HashVisitor::VisitFileScopeAsmDecl(const FileScopeAsmDecl *Node){
    Hash() << "FileScopeAsmDecl";

    const StringLiteral *sl = Node->getAsmString();
    if(sl != nullptr){
        hashStmt(sl);
    }
    return true;

}

//???? Stmt dazu wurde gemacht, also gibts dazu auch ne Decl
bool HashVisitor::VisitCapturedDecl(const CapturedDecl *Node){
    Hash() << "CapturedDecl";
    Stmt *body = Node->getBody();
    if(body != nullptr){
        hashStmt(body);
    }
    Hash() << Node->isNothrow();

    for(unsigned i = 0; i < Node->getNumParams();i++){
        ImplicitParamDecl *ipd = Node->getParam(i);
        if(ipd == nullptr){
            errs() << "nullptr in CapturedDecl!";
            exit(0);
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
    const sha1::digest * saved_digest = GetHash(attr);
    if(saved_digest){
        Hash() << *saved_digest;
        return;
    }


    // Visit in Pre-Order
    unsigned Depth = beforeDescent();

    const sha1::SHA1 *hash = PushHash();


    //No visitor exists. do it per hand
    //TODO ParmeterABIAttr & StmtAttr
    if(isa<InheritableParamAttr>(attr)){
        VisitInheritableParamAttr(  (InheritableParamAttr*) attr);
    }
    else if(isa<InheritableAttr>(attr)){
        VisitInheritableAttr(  (InheritableAttr*) attr);
    }
    else{
        VisitAttr(attr);
    }


    afterDescent(Depth);

    const sha1::digest digest = PopHash(hash);

    // Store hash for underlying type
    StoreHash(attr, digest);
}





//Attrs
//uncommented Attr not found in namespace
bool HashVisitor::VisitAttr(const Attr *attr){
    Hash() << "Attr";
    Hash() << attr->getKind();//hash enum
    Hash() << attr->isPackExpansion();


    return true;
}

bool HashVisitor::VisitInheritableAttr(const InheritableAttr *attr){
    Hash() << "Inheritable Attr";
    VisitAttr(attr);

    return true;
}
/*
bool HashVisitor::VisitStmtAttr(const StmtAttr *attr){
    Hash() << "Stmt Attr";
    VisitAttr(attr);

    return true;
}
*/

bool HashVisitor::VisitInheritableParamAttr(const InheritableParamAttr *attr){

    Hash() << "InheritableParamAttr";
    VisitAttr(attr);
    return true;
}

/*
bool HashVisitor::VisitParameterABIAttr(const ParameterABIAttr *attr){

    Hash() << "ParameterABAttr";
    const ParameterABI pabi = getABI();
    Hash() << pabi;//struct
    VisitAttr(attr);

    return true;
}
*/



//statements
void HashVisitor::hashStmt(const Stmt *stmt){
	//TODO: stimmt das so?
	unsigned depth = beforeDescent();
	const sha1::SHA1 *hash = PushHash();
	bool handled = mt_stmtvisitor::Visit(stmt);
	if(!handled){
		errs() << "---- START unhandled statement ----\n";
		stmt->dump();
		errs() << "----- END unhandled statement -----\n";
	}
	afterDescent(depth);
	const sha1::digest digest = PopHash(hash);
	Hash() << digest;
}

//TODO: default-case
bool HashVisitor::VisitStmt(const Stmt *Node)
{
    errs() << "StatementNotHandled";
    return false;
}

bool HashVisitor::VisitCompoundStmt(const CompoundStmt *stmt){
	Hash() << "compound";
	for(CompoundStmt::const_body_iterator iter = stmt->body_begin(); iter != stmt->body_end(); iter++){
		hashStmt(*iter);
	}
	return true;
}

bool HashVisitor::VisitBreakStmt(const BreakStmt *stmt){
	Hash() << "break";
	return true;
}

bool HashVisitor::VisitContinueStmt(const ContinueStmt *stmt){
	Hash() << "continue";
	return true;
}

bool HashVisitor::VisitGotoStmt(const GotoStmt *stmt){
	Hash() << "goto";
	hashDecl(stmt->getLabel());
	return true;
}

bool HashVisitor::VisitLabelStmt(const LabelStmt *stmt){
	Hash() << "label";
	Hash() << stmt->getName();
	hashStmt(stmt->getSubStmt());
	return true;
}

bool HashVisitor::VisitDoStmt(const DoStmt *stmt){
	Hash() << "do-while";
	hashStmt(stmt->getCond());
	hashStmt(stmt->getBody());
	return true;
}

bool HashVisitor::VisitForStmt(const ForStmt *stmt){
	Hash() << "for";
	if(stmt->getInit() != nullptr){
		hashStmt(stmt->getInit());
	}
	else{
		//TODO: hier irgendwas hashen?
	}

	if(stmt->getConditionVariable() != nullptr){
		hashStmt(stmt->getConditionVariableDeclStmt());
        }
	else{
		//TODO: hier irgendwas hashen?
	}

	if(stmt->getCond() != nullptr){
		hashStmt(stmt->getCond());
	}
	else{
		//TODO: hier irgendwas hashen?
	}

	if(stmt->getInc() != nullptr){
		hashStmt(stmt->getInc());
	}
	else{
		//TODO: hier irgendwas hashen?
	}

	hashStmt(stmt->getBody());
	return true;
}

bool HashVisitor::VisitIfStmt(const IfStmt *stmt){
	Hash() << "if";
	if(stmt->getConditionVariable() != nullptr){
		hashStmt(stmt->getConditionVariableDeclStmt());
	}
	else{
		//TODO: hier irgendwas hashen?
	}
	hashStmt(stmt->getCond());
	hashStmt(stmt->getThen());
	if(stmt->getElse() != nullptr){
		hashStmt(stmt->getElse());
	}
	else{
		//TODO: hier irgendwas hashen?
	}
	return true;
}

bool HashVisitor::VisitNullStmt(const NullStmt *stmt){
	//macht funktional keinen Unterschied...
	Hash() << "NullStmt";
	return true;
}

bool HashVisitor::VisitReturnStmt(const ReturnStmt *stmt){
	Hash() << "return";
	if(stmt->getRetValue() != nullptr){
		hashStmt(stmt->getRetValue());	
	}
	else{
		//TODO: hier irgendwas hashen?
	}
	return true;
}

bool HashVisitor::VisitWhileStmt(const WhileStmt *stmt){
	Hash() << "while";
	if(stmt->getConditionVariable() != nullptr){
		hashStmt(stmt->getConditionVariableDeclStmt());
	}
	else{
		//TODO: hier irgendwas hashen?
	}
	hashStmt(stmt->getCond());
	hashStmt(stmt->getBody());
	return true;
}

bool HashVisitor::VisitSwitchStmt(const SwitchStmt *stmt){
	Hash() << "switch";
	if(stmt->getConditionVariable() != nullptr){
		hashStmt(stmt->getConditionVariableDeclStmt());
	}
	else{
		//TODO: hier irgendwas hashen?
	}
	if(stmt->getCond() != nullptr){
		hashStmt(stmt->getCond());
	}
	else{
		//TODO: hier irgendwas hashen?
	}
	hashStmt(stmt->getBody());
	return true;
}

bool HashVisitor::VisitCaseStmt(const CaseStmt *stmt){
	Hash() << "case";
	hashStmt(stmt->getLHS());
	if(stmt->getRHS() != nullptr){
		hashStmt(stmt->getRHS());
	}
	else{
		//TODO: hier irgendwas hashen?
	}
	hashStmt(stmt->getSubStmt());
	return true;
}

bool HashVisitor::VisitDefaultStmt(const DefaultStmt *stmt){
	Hash() << "default";
	hashStmt(stmt->getSubStmt());
	return true;
}

bool HashVisitor::VisitDeclStmt(const DeclStmt *stmt){
	Hash() << "DeclStmt";
	for(DeclStmt::const_decl_iterator it = stmt->decl_begin(); it != stmt->decl_end(); it++){
		hashDecl(*it); 
	}
	return true;
}

bool HashVisitor::VisitGCCAsmStmt(const GCCAsmStmt *stmt){
	Hash() << "gcc asm";
	Hash() << stmt->getAsmString()->getString().str();
	return true;
}

bool HashVisitor::VisitMSAsmStmt(const MSAsmStmt *stmt){
	Hash() << "MS asm";
	Hash() << stmt->getAsmString().str();
	return true;
}

bool HashVisitor::VisitAttributedStmt(const AttributedStmt *stmt){
	Hash() << "AttributedStmt";
	for(const Attr * attr: stmt->getAttrs()){
        hashAttr(attr);
	}

	hashStmt(stmt->getSubStmt());
	return true;
}

bool HashVisitor::VisitCapturedStmt(const CapturedStmt *stmt){
	Hash() << "CaptureStmt";
	hashStmt(stmt->getCapturedStmt());
	hashDecl(stmt->getCapturedDecl());
	return true;
}

//not tested
bool HashVisitor::VisitSEHExceptStmt(const SEHExceptStmt *stmt){
	Hash() << "__except";
	hashStmt(stmt->getFilterExpr());
	hashStmt(stmt->getBlock());
	return true;
}

//not tested
bool HashVisitor::VisitSEHFinallyStmt(const SEHFinallyStmt *stmt){
	Hash() << "__finally";
	hashStmt(stmt->getBlock());
	return true;
}

//not tested
bool HashVisitor::VisitSEHLeaveStmt(const SEHLeaveStmt *stmt){
	Hash() << "__leave";
	return true;
}

//not tested
bool HashVisitor::VisitSEHTryStmt(const SEHTryStmt *stmt){
	Hash() << "__try";
	hashStmt(stmt->getTryBlock());
	hashStmt(stmt->getHandler());
	return true;
}

bool HashVisitor::VisitIndirectGotoStmt(const IndirectGotoStmt *stmt){
	errs() << "IndirectGotoStmt\n";
	exit(1);
}

bool HashVisitor::VisitOMPExecutableDirective(const OMPExecutableDirective *stmt){
	errs() << "OMPExecutableDirective";
	exit(1);
}
