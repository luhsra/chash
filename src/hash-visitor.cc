#undef NDEBUG

#include "hash-visitor.h"

#define MAGIC_NO 1

using namespace llvm;
using namespace clang;

typedef TranslationUnitHashVisitor HashVisitor;

std::string HashVisitor::getHash(unsigned *ProcessedBytes) {
  Hash::Digest TopLevelDigest = TopLevelHash.getDigest();
  if (ProcessedBytes) {
    *ProcessedBytes = TopLevelDigest.Length;
  }
  return TopLevelDigest.asString();
}

/// Declarations

// Only called, if Decl should be visited
void HashVisitor::hashDecl(const Decl *D) {
  if (!D) {
    return;
  }
  const Hash::Digest *const SavedDigest = getHash(D);
  if (SavedDigest) {
    topHash() << *SavedDigest;
    return;
  }

  // Visit in Pre-Order
  const unsigned Depth = beforeDescent();

  const Hash *const hash = pushHash();

  bool Handled = mt_declvisitor::Visit(D);
  if (!Handled) {
    errs() << "---- START unhandled -----\n";
    D->dump();
    errs() << "---- END unhandled -----\n";
  }

  // Decls within functions are visited by the body.
  if (!isa<FunctionDecl>(*D) && hasNodes(dyn_cast<DeclContext>(D)))
    hashDeclContext(cast<DeclContext>(D));

  // visit Attributes of the Decl
  for (const Attr *const attr : D->attrs()) {
    hashAttr(attr);
  }

  afterDescent(Depth);

  const Hash::Digest digest = popHash(hash);

  // Do not store or hash if flag is set
  if (DoNotHashThis) {
    DoNotHashThis = false;
    return;
  }

  // Store hash for underlying type
  storeHash(D, digest);

  // Hash into Parent
  if (!isa<TranslationUnitDecl>(D)) {
    topHash() << digest;
  }
}

bool HashVisitor::hasNodes(const DeclContext *DC) {
  if (!DC)
    return false;

  return DC->hasExternalLexicalStorage() ||
         DC->noload_decls_begin() != DC->noload_decls_end();
}

void HashVisitor::hashDeclContext(const DeclContext *DC) {
  if (!DC)
    return;

  for (auto *D : DC->noload_decls()) {
    // We don't need typedefs, Enums and Records here
    // TODO: Do we need to exclude more?
    if (!(isa<TagDecl>(D) || isa<TypedefDecl>(D))) {
      hashDecl(D);
    }
  }
}

bool HashVisitor::VisitTranslationUnitDecl(const TranslationUnitDecl *Unit) {
  Hash *hash = pushHash();

  // FIXME Hash Compiler Options

  afterChildren([=] {
    storeHash(Unit, popHash(hash));
    TopLevelHash << *hash;
  });

  Unit->dump();

  return true;
}

bool HashVisitor::VisitVarDecl(const VarDecl *Decl) {
  // Ignore extern declarations
  if (Decl->hasExternalStorage()) {
    DoNotHashThis = true;
    return true;
  }
  haveSeen(Decl, Decl);

  topHash() << AstElementVarDecl;
  hashName(Decl);
  hashType(Decl->getType());

  topHash() << Decl->getStorageClass();
  topHash() << Decl->getTLSKind();
  topHash() << Decl->isModulePrivate();
  topHash() << Decl->isNRVOVariable();

  if (Decl->hasInit()) {
    const Expr *expr = Decl->getInit();
    topHash() << AstElementVarDecl_init;
    hashStmt(expr);
  }

  return true;
}

bool HashVisitor::VisitImplicitParamDecl(const ImplicitParamDecl *Node) {
  topHash() << AstElementImplicitParamDecl;
  VisitVarDecl(Node); // important stuff done by parent
  return true;
}

bool HashVisitor::VisitParmVarDecl(const ParmVarDecl *Node) {
  topHash() << AstElementParamVarDecl;
  if (Node->hasDefaultArg()) {
    hashStmt(Node->getDefaultArg());
  }
  hashType(Node->getOriginalType());
  topHash() << Node->isParameterPack();
  VisitVarDecl(Node); // visit Parent
  return true;
}

// Types
void HashVisitor::hashType(QualType T) {
  uint64_t Qualifiers = 0;
  if (T.hasQualifiers()) {
    // TODO evtl. CVRMASK benutzen
    if (T.isLocalConstQualified()) {
      Qualifiers |= 1;
    }
    if (T.isLocalRestrictQualified()) {
      Qualifiers |= (1 << 1);
    }
    if (T.isLocalVolatileQualified()) {
      Qualifiers |= (1 << 2);
    }
    // weitere qualifier?
  }

  const Type *type = T.getTypePtr();
  assert(type != nullptr);

  if (Qualifiers) {
    topHash() << Qualifiers;
  }
  // T->dump();
  // errs() << type<< " " << qualifiers << " qualifiers\n";

  const Hash::Digest *SavedDigest = getHash(type);

  if (SavedDigest) {
    topHash() << *SavedDigest;
    return;
  }

  if (type->isStructureType()) {
    if (haveSeen(type, type)) {
      topHash() << "struct";
      topHash() << T.getAsString();
      return;
    }
  } else if (type->isUnionType()) {
    if (haveSeen(type, type)) {
      topHash() << "union";
      topHash() << T.getAsString();
      return;
    }
  }

  // Visit in Pre-Order
  unsigned Depth = beforeDescent();
  const Hash *hash = pushHash();

  bool Handled = mt_typevisitor::Visit(type);
  if (!Handled) {
    errs() << "---- START unhandled type -----\n";
    type->dump();
    errs() << "---- END unhandled type -----\n";
  }

  afterDescent(Depth);

  const Hash::Digest digest = popHash(hash);

  // if(saved_digest && digest != *saved_digest){
  //	errs() << "Different hashes for\n";
  //	T->dump();
  //	errs() << "\t(saved hash for)\n";
  //	type->dump();
  //}

  assert((!SavedDigest || digest == *SavedDigest) && "Hashes do not match");
  // Hash into Parent
  topHash() << digest;

  // Store hash for underlying type
  storeHash(type, digest);
}

bool HashVisitor::VisitBuiltinType(const BuiltinType *T) {
  topHash() << T->getKind();
  assert(!T->isSugared());
  return true;
}

bool HashVisitor::VisitPointerType(const PointerType *T) {
  topHash() << "pointer";
  hashType(T->getPointeeType());
  return true;
}

bool HashVisitor::VisitArrayType(const ArrayType *T) {
  topHash() << "ArrayType";
  hashType(T->getElementType());
  topHash() << "["
            << "*"
            << "]";
  return true;
}

bool HashVisitor::VisitConstantArrayType(const ConstantArrayType *T) {
  topHash() << "ConstantArrayType";
  hashType(T->getElementType());
  topHash() << "[" << T->getSize().getZExtValue() << "]";
  return true;
}

bool HashVisitor::VisitVariableArrayType(const VariableArrayType *T) {
  topHash() << "VariableArrayType";
  hashType(T->getElementType());
  topHash() << "[";
  hashStmt(T->getSizeExpr());
  topHash() << "]";
  return true;
}

bool HashVisitor::VisitTypedefType(const TypedefType *T) {
  return mt_typevisitor::Visit(T->desugar().getTypePtr());
}

bool HashVisitor::VisitComplexType(const ComplexType *T) {
  topHash() << "complex";
  hashType(T->getElementType());
  return true;
}

bool HashVisitor::VisitAtomicType(const AtomicType *T) {
  topHash() << "atomic";
  hashType(T->getValueType());
  return true;
}

bool HashVisitor::VisitTypeOfExprType(const TypeOfExprType *T) {
  topHash() << "typeof";
  hashType(T->desugar());
  return true;
}

bool HashVisitor::VisitTypeOfType(const TypeOfType *T) {
  topHash() << "typeoftypetype";
  hashType(T->desugar());
  return true;
}

bool HashVisitor::VisitParenType(const ParenType *T) {
  topHash() << "parenType";
  hashType(T->desugar());
  return true;
}

bool HashVisitor::VisitFunctionType(const FunctionType *T) {
  topHash() << "functype";
  hashType(T->getReturnType());
  topHash() << T->getRegParmType();
  topHash() << T->getCallConv();
  return true;
}

bool HashVisitor::VisitFunctionProtoType(const FunctionProtoType *T) {
  topHash() << "funcprototype";
  hashType(T->getReturnType());
  for (QualType qt : T->getParamTypes()) {
    hashType(qt);
  }
  topHash() << T->getRegParmType();
  topHash() << T->getCallConv();
  return true;
}

bool HashVisitor::VisitEnumType(const EnumType *Node) {
  topHash() << "Enum Type";
  if (Node->isSugared()) {
    hashType(Node->desugar());
  }

  EnumDecl *ed = Node->getDecl();
  hashType(ed->getIntegerType());
  hashType(ed->getPromotionType());

  for (EnumConstantDecl *ecd : ed->enumerators()) {
    hashStmt(ecd->getInitExpr());
    topHash() << ecd->getInitVal().getExtValue();
    hashName(ecd);
  }
  hashName(ed);

  return true;
}

bool HashVisitor::VisitTagType(const TagType *Node) {
  topHash() << "Tag Type";
  hashDecl(Node->getDecl());
  return true;
}

bool HashVisitor::VisitAttributedType(const AttributedType *Node) {
  topHash() << "AttributedType";
  topHash() << Node->getAttrKind();
  hashType(Node->getModifiedType());
  hashType(Node->getEquivalentType());
  return true;
}

bool HashVisitor::VisitUnaryTransformType(const UnaryTransformType *T) {
  topHash() << "UnaryTransformType";
  hashType(T->getBaseType());
  hashType(T->getUnderlyingType());
  topHash() << T->getUTTKind();
  return true;
}

bool HashVisitor::VisitDecayedType(const DecayedType *T) {
  topHash() << "DecayedType";
  hashType(T->getOriginalType());
  hashType(T->getAdjustedType());
  hashType(T->getPointeeType());
  return true;
}

bool HashVisitor::VisitAdjustedType(const AdjustedType *T) {
  topHash() << "AdjustedType";
  hashType(T->getOriginalType());
  hashType(T->getAdjustedType());
  return true;
}

bool HashVisitor::VisitElaboratedType(const ElaboratedType *T) {
  topHash() << "ElaboratedType";
  hashType(T->getNamedType());
  topHash() << T->getKeyword();
  return true;
}

bool HashVisitor::VisitType(const Type *T) {
  const Hash::Digest *digest = getHash(T);
  if (digest) {
    topHash() << *digest;
    return true;
  }

  if (T->isStructureType()) {
    haveSeen(T, T);
    topHash() << "struct";
    const RecordType *rt = T->getAsStructureType();
    RecordDecl *rd = rt->getDecl();

    // visit Attributes of the Decl (needed because RecordDecl shouldn't be not
    // called from hashDecl)
    for (Attr *attr : rd->attrs()) {
      hashAttr(attr);
    }

    for (RecordDecl::field_iterator Iter =
             rd->field_begin(); // TODO: put end in initialisation; condition:
                                // iter != e
         Iter != rd->field_end();
         ++Iter) {
      FieldDecl fd = **Iter;
      topHash() << "member";
      hashType(fd.getType());
      hashName(&fd);
    }
    return true;
  }

  if (T->isUnionType()) {
    haveSeen(T, T);
    topHash() << "union";
    const RecordType *rt = T->getAsUnionType();
    RecordDecl *rd = rt->getDecl();

    // visit Attributes of the Decl (needed because RecordDecl shouldn't be not
    // called from hashDecl)
    for (Attr *attr : rd->attrs()) {
      hashAttr(attr);
    }

    for (RecordDecl::field_iterator Iter =
             rd->field_begin(); // TODO: put end in initialisation; condition:
                                // iter != e
         Iter != rd->field_end();
         ++Iter) {
      FieldDecl fd = **Iter;
      topHash() << "member";
      hashType(fd.getType());
      hashName(&fd);
    }
    return true;
  }
  return false;
}

// Other Utilities
void HashVisitor::hashName(const NamedDecl *ND) {
  if (ND->getIdentifier() && ND->getDeclName()) {
    topHash() << ND->getNameAsString();
  } else {
    topHash() << 0;
  }
}

// Expressions
bool HashVisitor::VisitCastExpr(const CastExpr *Node) {
  topHash() << AstElementCastExpr;
  hashStmt(Node->getSubExpr());
  topHash() << Node->getCastKind();
  hashType(Node->getType());
  return true;
}

bool HashVisitor::VisitDeclRefExpr(const DeclRefExpr *Node) {
  const ValueDecl *const Decl = Node->getDecl();
  topHash() << "ref";
  // FIXME:	spaeter Auskommentiertes(isa-Zeug) einkommentieren (oder Problem
  // anders beheben)
  //		Andere Decls mehrfach referenzieren => Problem
  //		evtl. auch Zyklus in anderer Decl
  if (/*(isa<VarDecl>(Decl) || isa<FunctionDecl>(Decl)) && */ haveSeen(Decl,
                                                                       Decl)) {
    const Hash::Digest *digest = getHash(Decl);
    if (digest) {
      topHash() << *digest;
    } else {
      // if(!isa<VarDecl>(Decl) && !isa<FunctionDecl>(Decl)){
      //	errs() << "Not a VarDecl or FunctionDecl:\n";
      //	Decl->dump();
      //}
      assert(isa<VarDecl>(Decl) || isa<FunctionDecl>(Decl));
      if (isa<VarDecl>(Decl)) {
        VarDecl *vd = const_cast<VarDecl *>(static_cast<const VarDecl *>(Decl));
        topHash() << "VarDeclDummy";
        dummyVarDecl(vd);
      } else {
        FunctionDecl *fd =
            const_cast<FunctionDecl *>(static_cast<const FunctionDecl *>(Decl));
        dummyFunctionDecl(fd);
      }
    }
  } else {
    hashDecl(Decl);
  }
  hashName(Node->getFoundDecl());
  return true;
}

bool HashVisitor::VisitPredefinedExpr(const PredefinedExpr *Node) {
  topHash() << "predef";
  hashType(Node->getType());
  topHash() << Node->getFunctionName()->getString().str();
  return true;
}

bool HashVisitor::VisitCharacterLiteral(const CharacterLiteral *Node) {
  topHash() << AstElementCharacterLiteral;
  hashType(Node->getType());
  topHash() << Node->getValue();
  return true;
}

bool HashVisitor::VisitIntegerLiteral(const IntegerLiteral *Node) {
  topHash() << AstElementIntegerLiteral;
  hashType(Node->getType());
  if (Node->getValue().isNegative()) {
    topHash() << Node->getValue().getSExtValue();
  } else {
    topHash() << Node->getValue().getZExtValue();
  }
  return true;
}

bool HashVisitor::VisitFloatingLiteral(const FloatingLiteral *Node) {
  topHash() << AstElementFloatingLiteral;
  hashType(Node->getType());
  double mkay = (Node->getValue().convertToDouble()); //TODO: rename
  double *mmkay = &mkay;
  uint64_t *mval = reinterpret_cast<uint64_t *>(mmkay);
  uint64_t val = *mval;
  topHash() << val;
  return true;
}

bool HashVisitor::VisitStringLiteral(const StringLiteral *Str) {
  topHash() << AstElementStringLiteral;
  hashType(Str->getType());
  topHash() << Str->getString().str();
  return true;
}

bool HashVisitor::VisitInitListExpr(const InitListExpr *ILE) {
  topHash() << "initlistexpr";
  for (unsigned int i = 0; i < ILE->getNumInits(); ++i) {
    hashStmt(ILE->getInit(i));
  }
  if (ILE->hasArrayFiller()) {
    hashStmt(ILE->getArrayFiller());
  }
  return true;
}

bool HashVisitor::VisitUnaryOperator(const UnaryOperator *Node) {
  topHash() << "unary";
  hashStmt(Node->getSubExpr());
  topHash() << Node->getOpcode();
  hashType(Node->getType());
  return true;
}

bool HashVisitor::VisitUnaryExprOrTypeTraitExpr(
    const UnaryExprOrTypeTraitExpr *Node) {
  topHash() << "UOTT";
  topHash() << Node->getKind();
  if (Node->isArgumentType()) {
    hashType(Node->getArgumentType());
  } else {
    hashStmt(Node->getArgumentExpr());
  }
  return true;
}

bool HashVisitor::VisitMemberExpr(const MemberExpr *Node) {
  topHash() << "member";
  Expr *Base = Node->getBase();
  hashStmt(Base);

  ValueDecl *Member = Node->getMemberDecl();
  hashDecl(Member);

  topHash() << Node->isArrow();

  return true;
}

bool HashVisitor::VisitBinaryOperator(const BinaryOperator *Node) {
  topHash() << "binary";
  hashStmt(Node->getLHS());
  hashStmt(Node->getRHS());
  topHash() << Node->getOpcode();
  hashType(Node->getType());
  return true;
}

// erbt von BinaryOperator
bool
HashVisitor::VisitCompoundAssignOperator(const CompoundAssignOperator *Node) {
  topHash() << "compound";
  hashStmt(Node->getLHS());
  hashStmt(Node->getRHS());
  topHash() << Node->getOpcode();
  hashType(Node->getType());
  return true;
}

bool HashVisitor::VisitAddrLabelExpr(const AddrLabelExpr *Node) {
  topHash() << "addrlabel";
  hashStmt(Node->getLabel()->getStmt());
  return true;
}

bool HashVisitor::VisitImaginaryLiteral(const ImaginaryLiteral *Node) {
  topHash() << "imglit";
  hashType(Node->getType());
  hashStmt(Node->getSubExpr());
  return true;
}

bool HashVisitor::VisitCompoundLiteralExpr(const CompoundLiteralExpr *Node) {
  topHash() << "complit";
  hashType(Node->getType());
  hashStmt(Node->getInitializer());
  return true;
}

bool HashVisitor::VisitAbstractConditionalOperator(
    const AbstractConditionalOperator *Node) {
  topHash() << "ACondO";
  hashType(Node->getType());
  hashStmt(Node->getCond());
  hashStmt(Node->getTrueExpr());
  hashStmt(Node->getFalseExpr());
  return true;
}

bool HashVisitor::VisitBinaryConditionalOperator(
    const BinaryConditionalOperator *Node) {
  topHash() << "BCondO";
  hashType(Node->getType());
  hashStmt(Node->getCond());
  hashStmt(Node->getCommon());
  hashStmt(Node->getTrueExpr());
  hashStmt(Node->getFalseExpr());
  return true;
}

bool HashVisitor::VisitCallExpr(const CallExpr *Node) {
  topHash() << "callExpr";
  hashType(Node->getType());
  const FunctionDecl *fd = Node->getDirectCallee();
  if (fd) {
    hashName(fd);
  } else {
    hashStmt(Node->getCallee());
  }
  for (unsigned int i = 0; i < Node->getNumArgs(); ++i) {
    hashStmt(Node->getArg(i));
  }
  return true;
}

bool HashVisitor::VisitOffsetOfExpr(const OffsetOfExpr *Node) {
  topHash() << "offsetof";
  hashType(Node->getType());
  for (unsigned int i = 0; i < Node->getNumExpressions(); ++i) {
    hashStmt(Node->getIndexExpr(i));
  }
  for (unsigned int i = 0; i < Node->getNumComponents(); ++i) {
    OffsetOfNode off = Node->getComponent(i);
    topHash() << "offsetnode";
    topHash() << off.getKind();
    FieldDecl *fd;

    if (off.getKind() == OffsetOfNode::Kind::Field) {
      fd = off.getField();
      QualType t = fd->getType();
      hashType(t);

      hashName(off.getField());
    }
  }
  return true;
}

bool HashVisitor::VisitParenExpr(const ParenExpr *Node) {
  topHash() << "parenExpr";
  hashType(Node->getType());
  hashStmt(Node->getSubExpr());
  return true;
}

bool HashVisitor::VisitAtomicExpr(const AtomicExpr *Node) {
  AtomicExpr *MutableNode = const_cast<AtomicExpr *>(Node);

  topHash() << "atomicExpr";
  hashType(Node->getType());
  Expr **SubExprs = MutableNode->getSubExprs();
  for (unsigned int i = 0; i < MutableNode->getNumSubExprs(); ++i) {
    hashStmt(SubExprs[i]);
  }
  return true;
}

bool HashVisitor::VisitParenListExpr(const ParenListExpr *Node) {
  topHash() << "parenListExpr";
  hashType(Node->getType());
  for (Expr *expr : const_cast<ParenListExpr *>(Node)->exprs()) {
    hashStmt(expr);
  }
  return true;
}

bool HashVisitor::VisitDesignatedInitExpr(const DesignatedInitExpr *Node) {
  DesignatedInitExpr *MutableNode = const_cast<DesignatedInitExpr *>(Node);

  topHash() << "designatedInit";
  hashType(Node->getType());
  for (unsigned int i = 0; i < MutableNode->getNumSubExprs(); ++i) {
    hashStmt(MutableNode->getSubExpr(i));
  }
  for (unsigned int i = 0; i < MutableNode->size(); ++i) {
    DesignatedInitExpr::Designator *des = MutableNode->getDesignator(i);
    topHash() << "designator";
    hashType(des->getField()->getType());
    hashName(des->getField());
  }
  return true;
}

bool HashVisitor::VisitStmtExpr(const StmtExpr *Node) {
  topHash() << AstElementStmtExpr;
  hashType(Node->getType());
  hashStmt(Node->getSubStmt());
  return true;
}

bool HashVisitor::VisitArraySubscriptExpr(const ArraySubscriptExpr *Node) {
  topHash() << "ArrayAccess";
  hashStmt(Node->getLHS());
  hashStmt(Node->getRHS());
  hashStmt(Node->getBase());
  hashStmt(Node->getIdx());
  return true;
}

bool
HashVisitor::VisitImplicitValueInitExpr(const ImplicitValueInitExpr *Node) {
  topHash() << "implicitInit";
  hashType(Node->getType());
  return true;
}

bool HashVisitor::VisitVAArgExpr(const VAArgExpr *Node) {
  topHash() << "va_stuff";
  hashType(Node->getType());
  hashStmt(Node->getSubExpr());
  topHash() << Node->isMicrosoftABI();
  return true;
}

bool HashVisitor::VisitBlockExpr(const BlockExpr *Node) {
  topHash() << "block expr";
  hashDecl(Node->getBlockDecl());
  hashStmt(Node->getBody());

  return true;
}

// common Decls

bool HashVisitor::VisitBlockDecl(const BlockDecl *Node) {
  topHash() << "blockDecl";

  for (ParmVarDecl *par : Node->parameters()) {
    VisitVarDecl(par);
  }

  hashStmt(Node->getBody());
  return true;
}

bool HashVisitor::VisitFunctionDecl(const FunctionDecl *Node) {
  // Ignore extern declarations
  if (Node->getStorageClass() == StorageClass::SC_Extern ||
      Node->getStorageClass() == StorageClass::SC_PrivateExtern ||
      !Node->isThisDeclarationADefinition()) {
    DoNotHashThis = true;
    return true;
  }

  haveSeen(Node, Node);

  topHash() << "FunctionDecl";
  topHash() << Node->getNameInfo().getName().getAsString();
  hashStmt(Node->getBody());
  topHash() << Node->isDefined();
  topHash() << Node->isThisDeclarationADefinition();
  topHash() << Node->isVariadic();
  topHash() << Node->isVirtualAsWritten();
  topHash() << Node->isPure();
  topHash() << Node->hasImplicitReturnZero();
  topHash() << Node->hasPrototype();
  topHash() << Node->hasWrittenPrototype();
  topHash() << Node->hasInheritedPrototype();
  topHash() << Node->isMain();
  topHash() << Node->isExternC();
  topHash() << Node->isGlobal();
  topHash() << Node->isNoReturn();
  topHash() << Node->hasSkippedBody(); //???
  topHash() << Node->getBuiltinID();

  topHash() << Node->getStorageClass(); // static and stuff
  topHash() << Node->isInlineSpecified();
  topHash() << Node->isInlined();

  // hash all parameters
  for (ParmVarDecl *decl : Node->parameters()) {
    hashDecl(decl);
  }

  // vielleicht will man das ja auch:
  for (NamedDecl *decl : Node->getDeclsInPrototypeScope()) {
    hashDecl(decl);
  }

  // visit QualType
  hashType(Node->getReturnType());

  // here an error (propably nullptr) occured
  const IdentifierInfo *Ident = Node->getLiteralIdentifier();
  if (Ident) {
    const char *IdentName = Ident->getNameStart();
    if (IdentName)
      topHash() << IdentName;
  }

  topHash() << Node->getMemoryFunctionKind(); // maybe needed

  return true;
}

bool HashVisitor::VisitLabelDecl(const LabelDecl *Node) {
  topHash() << "labeldecl";
  hashName(Node);
  // if location changes, then it will be recompiled there.
  // Additionally the linker has this information--> no need to handle
  // this here (SourceRange)

  topHash() << Node->isGnuLocal();
  topHash() << Node->isMSAsmLabel();
  if (Node->isMSAsmLabel()) {
    topHash() << Node->getMSAsmLabel().str();
  }
  return true;
}

bool HashVisitor::VisitEnumDecl(const EnumDecl *Node) {
  const Hash *hash = pushHash();
  topHash() << "EnumDecl";
  hashName(Node);
  bool Handled = true;
  for (EnumConstantDecl *ecd : Node->enumerators()) {
    Handled &= mt_declvisitor::Visit(ecd);
    storeHash(ecd, topHash().getDigest());
  }
  popHash(hash);
  return Handled;
}
bool HashVisitor::VisitEnumConstantDecl(const EnumConstantDecl *Node) {
  topHash() << "EnumConstant";
  const Expr *expr = Node->getInitExpr();
  hashName(Node);
  if (expr) {
    hashStmt(expr);
  }
  topHash() << Node->getInitVal().getExtValue();

  return true;
}

// An instance of this class is created to represent a field injected from
// an anonymous union/struct into the parent scope
//--> do not follow the struct because it does not exist then...
bool HashVisitor::VisitIndirectFieldDecl(const IndirectFieldDecl *Node) {
  topHash() << "VisitIndirectFieldDecl";
  for (IndirectFieldDecl::chain_iterator Iter = Node->chain_begin();
       Iter != Node->chain_end();
       ++Iter) { // TODO: put end in initialisation; condition: iter != e
    NamedDecl nd = **Iter;
    hashDecl(&nd);
  }
  VisitValueDecl(Node);
  return true;
}

// called by children
bool HashVisitor::VisitValueDecl(const ValueDecl *Node) {

  topHash() << "VisitValueDecl";
  hashType(Node->getType());
  hashName(Node);
  return true;
}

bool HashVisitor::VisitFileScopeAsmDecl(const FileScopeAsmDecl *Node) {
  topHash() << "FileScopeAsmDecl";

  const StringLiteral *sl = Node->getAsmString();
  if (sl) {
    hashStmt(sl);
  }
  return true;
}

bool HashVisitor::VisitCapturedDecl(const CapturedDecl *Node) {
  topHash() << "CapturedDecl";
  Stmt *body = Node->getBody();
  if (body) {
    hashStmt(body);
  }
  topHash() << Node->isNothrow();

  for (unsigned i = 0; i < Node->getNumParams(); ++i) {
    ImplicitParamDecl *ipd = Node->getParam(i);
    if (!ipd) {
      errs() << "nullptr in CapturedDecl!";
      exit(1);
    }
    hashDecl(ipd);
  }

  return true;
}

// similar to hashDecl...
void HashVisitor::hashAttr(const Attr *attr) {
  if (!attr) {
    return;
  }
  // No visitor exists. do it per hand
  // TODO ParmeterABIAttr & StmtAttr (siehe uncommented Attr...)
  if (isa<InheritableParamAttr>(attr)) {
    VisitInheritableParamAttr(static_cast<const InheritableParamAttr *>(attr));
  } else if (isa<InheritableAttr>(attr)) {
    VisitInheritableAttr(static_cast<const InheritableAttr *>(attr));
  } else {
    VisitAttr(attr);
  }
}

// Attrs
// uncommented Attr not found in namespace
bool HashVisitor::VisitAttr(const Attr *attr) {
  topHash() << "Attr";
  topHash() << attr->getKind(); // hash enum
  topHash() << attr->isPackExpansion();
  return true;
}

bool HashVisitor::VisitInheritableAttr(const InheritableAttr *attr) {
  topHash() << "Inheritable Attr";
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

bool HashVisitor::VisitInheritableParamAttr(const InheritableParamAttr *attr) {
  topHash() << "InheritableParamAttr";
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

// statements
void HashVisitor::hashStmt(const Stmt *Stmt) {
  // Falls wir Nullpointer reinstecken
  if (!Stmt) {
    return;
  }

  bool Handled = mt_stmtvisitor::Visit(Stmt);
  if (!Handled) {
    errs() << "---- START unhandled statement ----\n";
    Stmt->dump();
    errs() << "----- END unhandled statement -----\n";
  }
}

bool HashVisitor::VisitStmt(const Stmt *Node) {
  errs() << "Statement Not Handled\n";
  return false;
}

bool HashVisitor::VisitCompoundStmt(const CompoundStmt *Stmt) {
  topHash() << "compound";
  for (CompoundStmt::const_body_iterator Iter = Stmt->body_begin();
       Iter != Stmt->body_end();
       ++Iter) { // TODO: put end in initialisation; condition: iter != e
    hashStmt(*Iter);
  }
  return true;
}

bool HashVisitor::VisitBreakStmt(const BreakStmt *Stmt) {
  topHash() << "break";
  return true;
}

bool HashVisitor::VisitContinueStmt(const ContinueStmt *Stmt) {
  topHash() << AstElementContinueStmt;
  return true;
}

bool HashVisitor::VisitGotoStmt(const GotoStmt *Stmt) {
  topHash() << AstElementGotoStmt;
  hashDecl(Stmt->getLabel());
  return true;
}

bool HashVisitor::VisitLabelStmt(const LabelStmt *Stmt) {
  topHash() << AstElementLabelStmt;
  hashDecl(Stmt->getDecl());
  hashStmt(Stmt->getSubStmt());
  return true;
}

bool HashVisitor::VisitDoStmt(const DoStmt *Stmt) {
  topHash() << AstElementDoStmt;
  hashStmt(Stmt->getCond());
  hashStmt(Stmt->getBody());
  return true;
}

bool HashVisitor::VisitForStmt(const ForStmt *Stmt) {
  topHash() << AstElementForStmt;
  hashStmt(Stmt->getInit());
  hashStmt(Stmt->getConditionVariableDeclStmt());
  hashStmt(Stmt->getCond());
  hashStmt(Stmt->getInc());
  hashStmt(Stmt->getBody());
  return true;
}

bool HashVisitor::VisitIfStmt(const IfStmt *Stmt) {
  topHash() << AstElementIfStmt;
  hashStmt(Stmt->getConditionVariableDeclStmt());
  hashStmt(Stmt->getCond());
  hashStmt(Stmt->getThen());
  hashStmt(Stmt->getElse());
  return true;
}

bool HashVisitor::VisitNullStmt(const NullStmt *Stmt) {
  // macht funktional keinen Unterschied...
  topHash() << AstElementNullStmt;
  return true;
}

bool HashVisitor::VisitReturnStmt(const ReturnStmt *Stmt) {
  topHash() << AstElementReturnStmt;
  hashStmt(Stmt->getRetValue());
  return true;
}

bool HashVisitor::VisitWhileStmt(const WhileStmt *Stmt) {
  topHash() << AstElementWhileStmt;
  hashStmt(Stmt->getConditionVariableDeclStmt());
  hashStmt(Stmt->getCond());
  hashStmt(Stmt->getBody());
  return true;
}

bool HashVisitor::VisitSwitchStmt(const SwitchStmt *Stmt) {
  topHash() << AstElementSwitchStmt;
  hashStmt(Stmt->getConditionVariableDeclStmt());
  hashStmt(Stmt->getCond());
  hashStmt(Stmt->getBody());
  return true;
}

bool HashVisitor::VisitCaseStmt(const CaseStmt *Stmt) {
  topHash() << AstElementCaseStmt;
  hashStmt(Stmt->getLHS());
  hashStmt(Stmt->getRHS());
  hashStmt(Stmt->getSubStmt());
  return true;
}

bool HashVisitor::VisitDefaultStmt(const DefaultStmt *Stmt) {
  topHash() << AstElementDefaultStmt;
  hashStmt(Stmt->getSubStmt());
  return true;
}

bool HashVisitor::VisitDeclStmt(const DeclStmt *Stmt) {
  topHash() << AstElementDeclStmt;
  for (DeclStmt::const_decl_iterator it = Stmt->decl_begin();
       it != Stmt->decl_end();
       ++it) { // TODO: put end in initialisation; condition: iter != e
    hashDecl(*it);
  }
  return true;
}

bool HashVisitor::VisitGCCAsmStmt(const GCCAsmStmt *Stmt) {
  topHash() << "gcc asm";
  topHash() << Stmt->getAsmString()->getString().str();
  return true;
}

bool HashVisitor::VisitMSAsmStmt(const MSAsmStmt *Stmt) {
  topHash() << "MS asm";
  topHash() << Stmt->getAsmString().str();
  return true;
}

bool HashVisitor::VisitAttributedStmt(const AttributedStmt *Stmt) {
  topHash() << "AttributedStmt";
  for (const Attr *attr : Stmt->getAttrs()) {
    hashAttr(attr);
  }

  hashStmt(Stmt->getSubStmt());
  return true;
}

bool HashVisitor::VisitCapturedStmt(const CapturedStmt *Stmt) {
  topHash() << "CaptureStmt";
  hashStmt(Stmt->getCapturedStmt());
  hashDecl(Stmt->getCapturedDecl());
  return true;
}

// not tested
bool HashVisitor::VisitSEHExceptStmt(const SEHExceptStmt *Stmt) {
  topHash() << "__except";
  hashStmt(Stmt->getFilterExpr());
  hashStmt(Stmt->getBlock());
  return true;
}

// not tested
bool HashVisitor::VisitSEHFinallyStmt(const SEHFinallyStmt *Stmt) {
  topHash() << "__finally";
  hashStmt(Stmt->getBlock());
  return true;
}

// not tested
bool HashVisitor::VisitSEHLeaveStmt(const SEHLeaveStmt *Stmt) {
  topHash() << "__leave";
  return true;
}

// not tested
bool HashVisitor::VisitSEHTryStmt(const SEHTryStmt *Stmt) {
  topHash() << "__try";
  hashStmt(Stmt->getTryBlock());
  hashStmt(Stmt->getHandler());
  return true;
}

bool HashVisitor::VisitIndirectGotoStmt(const IndirectGotoStmt *Stmt) {
  topHash() << "IndirectGotoStmt";
  hashStmt(Stmt->getTarget());
  if (Stmt->getConstantTarget()) {
    hashDecl(Stmt->getConstantTarget());
  }
  return true;
}

// OpenMP directives, not tested
bool
HashVisitor::VisitOMPExecutableDirective(const OMPExecutableDirective *Stmt) {
  errs() << "OMPExecutableDirectives are not implemented yet.\n";
  exit(1);
}
