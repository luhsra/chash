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

	//TODO: static in Funktion schon abgedeckt?
    Hash() << Decl->getStorageClass();
    Hash() << Decl->getTLSKind();
    Hash() << Decl->isModulePrivate(); /* globales static */
    Hash() << Decl->isNRVOVariable();

   // FIXME Init Statement (vmtl. Zuweisung)

    return true;
}

// Types
void HashVisitor::hashType(QualType T) {
	uint64_t qualifiers = 0;
    if(T.hasQualifiers()){
		//TODO evtl. typedef indirektion evtl. CVRMASK benutzen
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

    bool handled = mt_typevisitor::Visit(type);
    if (!handled) {
        errs() << "---- START unhandled type -----\n";
        type->dump();
        errs() << "---- END unhandled type -----\n";

    }

    afterDescent(Depth);

    const sha1::digest digest = PopHash(hash);

	if(qualifiers) {
		Hash() << qualifiers;
	}

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
	//FIXME: FunctionPointerType
	//TODO: evtl. Zeug um visit rum
	if((T->getPointeeType()).getTypePtr()->isStructureType()){
		Hash() << "struct";
		Hash() << (T->getPointeeType()).getAsString();
		return true;
	}else if((T->getPointeeType()).getTypePtr()->isUnionType()){
		Hash() << "union";
		Hash() << (T->getPointeeType()).getAsString();
		return true;
	}else{
		return mt_typevisitor::Visit((T->getPointeeType()).getTypePtr());
	}
}

bool HashVisitor::VisitArrayType(const ArrayType *T){
	//TODO: evtl. Zeug um visit rum
	const sha1::SHA1 *hash = PushHash();
	bool iDidIt = mt_typevisitor::Visit(T->getElementType().getTypePtr());
	const sha1::digest digest = PopHash(hash);
	Hash() << digest;
	Hash() << "[" << "*" << "]";
	return iDidIt;
}

bool HashVisitor::VisitConstantArrayType(const ConstantArrayType *T){
	const sha1::SHA1 *hash = PushHash();
	bool iDidIt = mt_typevisitor::Visit(T->getElementType().getTypePtr());
	const sha1::digest digest = PopHash(hash);
	Hash() << digest;
	Hash() << "[" << T->getSize().getZExtValue() << "]";
	return iDidIt;
}

bool HashVisitor::VisitType(const Type *T){
	if(T->isStructureType()){
		Hash() << "struct";
		const RecordType *rt = T->getAsStructureType();
		RecordDecl *rd = rt->getDecl();
		const sha1::SHA1 *hash = PushHash();
		for(RecordDecl::field_iterator iter=rd->field_begin(); iter != rd->field_end(); iter++){
			FieldDecl fd = **iter;
			hashType(fd.getType());
			hashName(&fd);
		}
		const sha1::digest digest = PopHash(hash);
		Hash() << digest;
		return true;
		
	}else if(T->isUnionType()){
		Hash() << "union";
		const RecordType *rt = T->getAsUnionType();
		RecordDecl *rd = rt->getDecl();
		const sha1::SHA1 *hash = PushHash();
		for(RecordDecl::field_iterator iter=rd->field_begin(); iter != rd->field_end(); iter++){
			FieldDecl fd = **iter;
			hashType(fd.getType());
			hashName(&fd);
		}
		const sha1::digest digest = PopHash(hash);
		Hash() << digest;
		return true;
		
	}else{ //FIXME: FunctionPointerType
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
