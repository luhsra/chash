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

void HashVisitor::hashDecl(const Decl *D) {
    if (!D) {
        return;
    }

    // Visit in Pre-Order
    unsigned Depth = beforeDescent();

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
	hashType(T->getElementType());
	Hash() << "[" << "*" << "]";
	return true;
}

bool HashVisitor::VisitConstantArrayType(const ConstantArrayType *T){
	hashType(T->getElementType());
	Hash() << "[" << T->getSize().getZExtValue() << "]";
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

bool HashVisitor::VisitType(const Type *T){
	if(T->isStructureType()){
		Hash() << "struct";
		const RecordType *rt = T->getAsStructureType();
		RecordDecl *rd = rt->getDecl();
		for(RecordDecl::field_iterator iter=rd->field_begin(); iter != rd->field_end(); iter++){
			FieldDecl fd = **iter;
			hashType(fd.getType());
			hashName(&fd);
		}
		return true;
		
	}else if(T->isUnionType()){
		Hash() << "union";
		const RecordType *rt = T->getAsUnionType();
		RecordDecl *rd = rt->getDecl();
		for(RecordDecl::field_iterator iter=rd->field_begin(); iter != rd->field_end(); iter++){
			FieldDecl fd = **iter;
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
	const sha1::SHA1 *hash = PushHash();
	bool handled = mt_stmtvisitor::Visit(Node->getSubExpr());
	const sha1::digest digest = PopHash(hash);
	Hash() << "cast";
	Hash() << digest;
	Hash() << Node->getCastKind();
	hashType(Node->getType());
	return handled;	
}

bool HashVisitor::VisitDeclRefExpr(const DeclRefExpr *Node){
	const ValueDecl *vd = Node->getDecl();
	Hash() << "ref";
	hashType(vd->getType());
	hashName(vd);
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
	const sha1::SHA1 *hash = PushHash();
	bool handled = true;
	for(unsigned int i = 0; i < ILE->getNumInits(); i++){
		handled &= mt_stmtvisitor::Visit(ILE->getInit(i));
	}
	const sha1::digest digest = PopHash(hash);
	Hash() << "list";
	Hash() << digest;
	return handled;
}

bool HashVisitor::VisitUnaryOperator(const UnaryOperator *Node){
	const sha1::SHA1 *hash = PushHash();
	bool handled = mt_stmtvisitor::Visit(Node->getSubExpr());
	const sha1::digest digest = PopHash(hash);
	Hash() << "unary";
	Hash() << Node->getOpcode();
	Hash() << digest;
	hashType(Node->getType());
	return handled;
}

bool HashVisitor::VisitUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *Node){
	if(Node->isArgumentType()){
		Hash() << "UOTT";
		Hash() << Node->getKind();
		hashType(Node->getArgumentType());
		return true;
	}else{
		const sha1::SHA1 *hash = PushHash();
		bool handled = mt_stmtvisitor::Visit(Node->getArgumentExpr());
		const sha1::digest digest = PopHash(hash);
		Hash() << "UOTT";
		Hash() << Node->getKind();
		Hash() << digest;
		return handled;
	}
}

bool HashVisitor::VisitMemberExpr(const MemberExpr *Node){
	//TODO
    Hash() << "member";
    Expr base = Node->getBase();
    bool handled = true;
    handled &= mt_stmtvisitor::Visit(base);

    //hashType(Node->);
    ValueDecl *member = getMemberDecl();
    FieldDecl *real_member = dynamic_cast<FieldDecl*>(member());
    assert(real_member);//otherwise it is C++ and invalid
    handled &= mt_stmtvisitor::Visit(real_member);

    Hash() << Node->isArrow();



	return false;
}

bool HashVisitor::VisitBinaryOperator(const BinaryOperator *Node){
	const sha1::SHA1 *hash = PushHash();
	bool handled = mt_stmtvisitor::Visit(Node->getLHS());
	handled &= mt_stmtvisitor::Visit(Node->getRHS());
	const sha1::digest digest = PopHash(hash);
	Hash() << "binary";
	Hash() << Node->getOpcode();
	Hash() << digest;
	hashType(Node->getType());
	return handled;
}

//erbt von BinaryOperator
bool HashVisitor::VisitCompoundAssignOperator(const CompoundAssignOperator *Node){	
	const sha1::SHA1 *hash = PushHash();
	bool handled = mt_stmtvisitor::Visit(Node->getLHS());
	handled &= mt_stmtvisitor::Visit(Node->getRHS());
	const sha1::digest digest = PopHash(hash);
	Hash() << "compound";
	Hash() << Node->getOpcode();
	Hash() << digest;
	hashType(Node->getType());
	return handled;
}

bool HashVisitor::VisitAddrLabelExpr(const AddrLabelExpr *Node){
	const sha1::SHA1 *hash = PushHash();
	bool handled = mt_stmtvisitor::Visit(Node->getLabel()->getStmt());
	const sha1::digest digest = PopHash(hash);
	Hash() << "addrlabel";
	Hash() << digest;
	return handled;
}

bool HashVisitor::VisitBlockExpr(const BlockExpr *Node){
	//TODO
	return false;
}
