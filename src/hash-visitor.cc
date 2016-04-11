#undef NDEBUG

#include "hash-visitor.h"

#define MAGIC_NO 1

using namespace llvm;
using namespace clang;

typedef TranslationUnitHashVisitor HashVisitor;

std::string HashVisitor::getHash(unsigned *ProcessedBytes) {
  const Hash::Digest TopLevelDigest = TopLevelHash.getDigest();
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
  const Hash *const CurrentHash = pushHash();

  const bool Handled = mt_declvisitor::Visit(D);
  if (!Handled) {
    errs() << "---- START unhandled -----\n";
    D->dump();
    errs() << "---- END unhandled -----\n";
  }

  // Decls within functions are visited by the body.
  if (!isa<FunctionDecl>(*D) && hasNodes(dyn_cast<DeclContext>(D)))
    hashDeclContext(cast<DeclContext>(D));

  // Visit attributes of the decl
  for (const Attr *const A : D->attrs()) {
    hashAttr(A);
  }

  afterDescent(Depth);

  const Hash::Digest CurrentDigest = popHash(CurrentHash);

  // Do not store or hash if flag is set
  if (DoNotHashThis) {
    DoNotHashThis = false;
    return;
  }

  // Store hash for underlying type
  storeHash(D, CurrentDigest);

  // Hash into parent
  if (!isa<TranslationUnitDecl>(D)) {
    topHash() << CurrentDigest;
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

  for (const auto *const D : DC->noload_decls()) {
    // We don't need typedefs, enums and records here
    // TODO: Do we need to exclude more?
    if (!(isa<TagDecl>(D) || isa<TypedefDecl>(D))) {
      hashDecl(D);
    }
  }
}

bool HashVisitor::VisitTranslationUnitDecl(const TranslationUnitDecl *Unit) {
  const Hash *const CurrentHash = pushHash();

  // FIXME Hash Compiler Options

  afterChildren([=] {
    storeHash(Unit, popHash(CurrentHash));
    TopLevelHash << *CurrentHash;
  });

  Unit->dump();

  return true;
}

bool HashVisitor::VisitVarDecl(const VarDecl *D) {
  // Ignore extern declarations
  if (D->hasExternalStorage()) {
    DoNotHashThis = true;
    return true;
  }
  haveSeen(D, D);

  topHash() << AstElementVarDecl;
  hashName(D);
  hashType(D->getType());

  topHash() << D->getStorageClass();
  topHash() << D->getTLSKind();
  topHash() << D->isModulePrivate();
  topHash() << D->isNRVOVariable();

  if (D->hasInit()) {
    const Expr *const E = D->getInit();
    topHash() << AstElementVarDecl_init;
    hashStmt(E);
  }

  return true;
}

bool HashVisitor::VisitImplicitParamDecl(const ImplicitParamDecl *D) {
  topHash() << AstElementImplicitParamDecl;
  VisitVarDecl(D); // important stuff done by parent
  return true;
}

bool HashVisitor::VisitParmVarDecl(const ParmVarDecl *D) {
  topHash() << AstElementParamVarDecl;
  if (D->hasDefaultArg()) {
    hashStmt(D->getDefaultArg());
  }
  hashType(D->getOriginalType());
  topHash() << D->isParameterPack();
  VisitVarDecl(D); // visit Parent
  return true;
}

// Types
void HashVisitor::hashType(const QualType& T) {
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

  const Type *const ActualType = T.getTypePtr();
  assert(ActualType != nullptr);

  if (Qualifiers) {
    topHash() << Qualifiers;
  }
  // T->dump();
  // errs() << type<< " " << qualifiers << " qualifiers\n";

  const Hash::Digest *const SavedDigest = getHash(ActualType);

  if (SavedDigest) {
    topHash() << *SavedDigest;
    return;
  }

  if (ActualType->isStructureType()) {
    if (haveSeen(ActualType, ActualType)) {
      topHash() << "struct";
      topHash() << T.getAsString();
      return;
    }
  } else if (ActualType->isUnionType()) {
    if (haveSeen(ActualType, ActualType)) {
      topHash() << "union";
      topHash() << T.getAsString();
      return;
    }
  }

  // Visit in Pre-Order
  const unsigned Depth = beforeDescent();
  const Hash *const CurrentHash = pushHash();

  bool Handled = mt_typevisitor::Visit(ActualType);
  if (!Handled) {
    errs() << "---- START unhandled type -----\n";
    ActualType->dump();
    errs() << "---- END unhandled type -----\n";
  }

  afterDescent(Depth);

  const Hash::Digest CurrentDigest = popHash(CurrentHash);

  // if(saved_digest && digest != *saved_digest){
  //	errs() << "Different hashes for\n";
  //	T->dump();
  //	errs() << "\t(saved hash for)\n";
  //	type->dump();
  //}

  assert((!SavedDigest || CurrentDigest == *SavedDigest) &&
         "Hashes do not match");
  // Hash into Parent
  topHash() << CurrentDigest;

  // Store hash for underlying type
  storeHash(ActualType, CurrentDigest);
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
  for (const QualType &QT : T->getParamTypes()) {
    hashType(QT);
  }
  topHash() << T->getRegParmType();
  topHash() << T->getCallConv();
  return true;
}

bool HashVisitor::VisitEnumType(const EnumType *T) {
  topHash() << "Enum Type";
  if (T->isSugared()) {
    hashType(T->desugar());
  }

  const EnumDecl *const ED = T->getDecl();
  hashType(ED->getIntegerType());
  hashType(ED->getPromotionType());

  for (const EnumConstantDecl *const ECD : ED->enumerators()) {
    hashStmt(ECD->getInitExpr());
    topHash() << ECD->getInitVal().getExtValue();
    hashName(ECD);
  }
  hashName(ED);

  return true;
}

bool HashVisitor::VisitTagType(const TagType *T) {
  topHash() << "Tag Type";
  hashDecl(T->getDecl());
  return true;
}

bool HashVisitor::VisitAttributedType(const AttributedType *T) {
  topHash() << "AttributedType";
  topHash() << T->getAttrKind();
  hashType(T->getModifiedType());
  hashType(T->getEquivalentType());
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
  if (const Hash::Digest *const D = getHash(T)) {
    topHash() << *D;
    return true;
  }

  // TODO: die 2 if-bloecke zusammenfassen; alles redundant!
  if (T->isStructureType()) {
    haveSeen(T, T);
    topHash() << "struct";
    const RecordType *const RT = T->getAsStructureType();
    const RecordDecl *const RD = RT->getDecl();

    // visit Attributes of the Decl (needed because RecordDecl shouldn't be not
    // called from hashDecl)
    for (const Attr *const A : RD->attrs()) {
      hashAttr(A);
    }

    for (const FieldDecl *const FD : RD->fields()) {
      topHash() << "member";
      hashType(FD->getType());
      hashName(FD);
    }
    return true;
  }

  if (T->isUnionType()) {
    haveSeen(T, T);
    topHash() << "union";
    const RecordType *const RT = T->getAsUnionType();
    const RecordDecl *const RD = RT->getDecl();

    // visit Attributes of the Decl (needed because RecordDecl shouldn't be not
    // called from hashDecl)
    // TODO: shouldn't be not called ??? => fix comment
    for (const Attr *const A : RD->attrs()) {
      hashAttr(A);
    }

    for (const FieldDecl *const FD : RD->fields()) {
      topHash() << "member";
      hashType(FD->getType());
      hashName(FD);
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
  const ValueDecl *const ValDecl = Node->getDecl();
  topHash() << "ref";
  // FIXME:	spaeter Auskommentiertes(isa-Zeug) einkommentieren (oder Problem
  // anders beheben)
  //		Andere Decls mehrfach referenzieren => Problem
  //		evtl. auch Zyklus in anderer Decl
  if (/*(isa<VarDecl>(Decl) || isa<FunctionDecl>(Decl)) && */ haveSeen(
      ValDecl, ValDecl)) {
    if (const Hash::Digest *const D = getHash(ValDecl)) {
      topHash() << *D;
    } else {
      // if(!isa<VarDecl>(Decl) && !isa<FunctionDecl>(Decl)){
      //	errs() << "Not a VarDecl or FunctionDecl:\n";
      //	Decl->dump();
      //}
      assert(isa<VarDecl>(ValDecl) || isa<FunctionDecl>(ValDecl));
      if (isa<VarDecl>(ValDecl)) {
        topHash() << "VarDeclDummy";
        const VarDecl *const VD = static_cast<const VarDecl *>(ValDecl);
        dummyVarDecl(VD);
      } else {
        const FunctionDecl *const FD =
            static_cast<const FunctionDecl *>(ValDecl);
        dummyFunctionDecl(FD);
      }
    }
  } else {
    hashDecl(ValDecl);
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
  const double Value = (Node->getValue().convertToDouble());
  topHash() << *reinterpret_cast<const uint64_t *>(&Value);
  return true;
}

bool HashVisitor::VisitStringLiteral(const StringLiteral *Node) {
  topHash() << AstElementStringLiteral;
  hashType(Node->getType());
  topHash() << Node->getString().str();
  return true;
}

bool HashVisitor::VisitInitListExpr(const InitListExpr *Node) {
  topHash() << "initlistexpr";
  for (unsigned I = 0, E = Node->getNumInits(); I < E; ++I) {
    hashStmt(Node->getInit(I));
  }
  if (Node->hasArrayFiller()) {
    hashStmt(Node->getArrayFiller());
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
  const Expr *const Base = Node->getBase();
  hashStmt(Base);

  const ValueDecl *const Member = Node->getMemberDecl();
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
  if (const FunctionDecl *const FD = Node->getDirectCallee()) {
    hashName(FD);
  } else {
    hashStmt(Node->getCallee());
  }
  for (unsigned I = 0, E = Node->getNumArgs(); I < E; ++I) {
    hashStmt(Node->getArg(I));
  }
  return true;
}

bool HashVisitor::VisitOffsetOfExpr(const OffsetOfExpr *Node) {
  topHash() << "offsetof";
  hashType(Node->getType());
  for (unsigned I = 0, E = Node->getNumExpressions(); I < E; ++I) {
    hashStmt(Node->getIndexExpr(I));
  }
  for (unsigned I = 0, E = Node->getNumComponents(); I < E; ++I) {
    const OffsetOfNode Offset = Node->getComponent(I);
    topHash() << "offsetnode";
    topHash() << Offset.getKind();

    if (Offset.getKind() == OffsetOfNode::Kind::Field) {
      const FieldDecl *const FD = Offset.getField();
      const QualType QT = FD->getType();
      hashType(QT);
      hashName(Offset.getField());
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
  AtomicExpr *const MutableNode = const_cast<AtomicExpr *>(Node);

  topHash() << "atomicExpr";
  hashType(Node->getType());
  Expr **SubExprs = MutableNode->getSubExprs();
  for (unsigned int I = 0, E = MutableNode->getNumSubExprs(); I < E; ++I) {
    hashStmt(SubExprs[I]);
  }
  return true;
}

bool HashVisitor::VisitParenListExpr(const ParenListExpr *Node) {
  topHash() << "parenListExpr";
  hashType(Node->getType());
  for (const Expr *const E : const_cast<ParenListExpr *>(Node)->exprs()) {
    hashStmt(E); // TODO: expr instead of stmt: ok or cast explicitly?
  }
  return true;
}

bool HashVisitor::VisitDesignatedInitExpr(const DesignatedInitExpr *Node) {
  DesignatedInitExpr *const MutableNode =
      const_cast<DesignatedInitExpr *>(Node);

  topHash() << "designatedInit";
  hashType(Node->getType());
  for (unsigned I = 0, E = MutableNode->getNumSubExprs(); I < E; ++I) {
    hashStmt(MutableNode->getSubExpr(I));
  }
  for (unsigned I = 0, E = MutableNode->size(); I < E; ++I) {
    DesignatedInitExpr::Designator *Des = MutableNode->getDesignator(I);
    topHash() << "designator";
    hashType(Des->getField()->getType());
    hashName(Des->getField());
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

bool HashVisitor::VisitBlockDecl(const BlockDecl *D) {
  topHash() << "blockDecl";
  for (const ParmVarDecl *const PVD : D->parameters()) {
    VisitVarDecl(PVD);
  }
  hashStmt(D->getBody());
  return true;
}

bool HashVisitor::VisitFunctionDecl(const FunctionDecl *Node) {
  // Ignore extern declarations
  if (Node->getStorageClass() == StorageClass::SC_Extern ||
      Node->getStorageClass() == StorageClass::SC_PrivateExtern ||
      !Node->isThisDeclarationADefinition()) { // TODO: move condition to own
                                               // method
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
  for (const ParmVarDecl *const PVD : Node->parameters()) {
    hashDecl(PVD);
  }

  // vielleicht will man das ja auch:
  for (const NamedDecl *const ND : Node->getDeclsInPrototypeScope()) {
    hashDecl(ND);
  }

  // visit QualType
  hashType(Node->getReturnType());

  // here an error (propably nullptr) occured
  if (const IdentifierInfo *const IdentInfo = Node->getLiteralIdentifier()) {
    if (const char *const IdentName = IdentInfo->getNameStart()) {
      topHash() << IdentName;
    }
  }

  topHash() << Node->getMemoryFunctionKind(); // maybe needed

  return true;
}

bool HashVisitor::VisitLabelDecl(const LabelDecl *D) {
  topHash() << "labeldecl";
  hashName(D);
  // if location changes, then it will be recompiled there.
  // Additionally the linker has this information--> no need to handle
  // this here (SourceRange)

  topHash() << D->isGnuLocal();
  topHash() << D->isMSAsmLabel();
  if (D->isMSAsmLabel()) {
    topHash() << D->getMSAsmLabel().str();
  }
  return true;
}

bool HashVisitor::VisitEnumDecl(const EnumDecl *D) {
  const Hash *const CurrentHash = pushHash();
  topHash() << "EnumDecl";
  hashName(D);
  bool Handled = true;
  for (const EnumConstantDecl *const ECD : D->enumerators()) {
    Handled &= mt_declvisitor::Visit(ECD);
    storeHash(ECD, topHash().getDigest());
  }
  popHash(CurrentHash);
  return Handled;
}
bool HashVisitor::VisitEnumConstantDecl(const EnumConstantDecl *D) {
  topHash() << "EnumConstant";
  hashName(D);
  if (const Expr *const InitExpr = D->getInitExpr()) {
    hashStmt(InitExpr);
  }
  topHash() << D->getInitVal().getExtValue();

  return true;
}

// An instance of this class is created to represent a field injected from
// an anonymous union/struct into the parent scope
//--> do not follow the struct because it does not exist then...
bool HashVisitor::VisitIndirectFieldDecl(const IndirectFieldDecl *D) {
  topHash() << "VisitIndirectFieldDecl";
  for (IndirectFieldDecl::chain_iterator I = D->chain_begin(),
                                         E = D->chain_end();
       I != E; ++I) {
    hashDecl(*I);
  }
  VisitValueDecl(D);
  return true;
}

// called by children
bool HashVisitor::VisitValueDecl(const ValueDecl *D) {
  topHash() << "VisitValueDecl";
  hashType(D->getType());
  hashName(D);
  return true;
}

bool HashVisitor::VisitFileScopeAsmDecl(const FileScopeAsmDecl *D) {
  topHash() << "FileScopeAsmDecl";
  if (const StringLiteral *const SL = D->getAsmString()) {
    hashStmt(SL);
  }
  return true;
}

bool HashVisitor::VisitCapturedDecl(const CapturedDecl *D) {
  topHash() << "CapturedDecl";
  if (const Stmt *const Body = D->getBody()) {
    hashStmt(Body);
  }
  topHash() << D->isNothrow();

  for (const ImplicitParamDecl *const IPD : D->params()) {
    if (!IPD) {
      errs() << "nullptr in CapturedDecl!";
      exit(1);
    }
    hashDecl(IPD);
  }
  return true;
}

// similar to hashDecl...
void HashVisitor::hashAttr(const Attr *A) {
  if (!A)
    return;

  // No visitor exists. do it per hand
  // TODO ParmeterABIAttr & StmtAttr (siehe uncommented Attr...)
  if (isa<InheritableParamAttr>(A)) {
    VisitInheritableParamAttr(static_cast<const InheritableParamAttr *>(A));
  } else if (isa<InheritableAttr>(A)) {
    VisitInheritableAttr(static_cast<const InheritableAttr *>(A));
  } else {
    VisitAttr(A);
  }
}

// Attrs
// uncommented Attr not found in namespace
bool HashVisitor::VisitAttr(const Attr *A) {
  topHash() << "Attr";
  topHash() << A->getKind(); // hash enum
  topHash() << A->isPackExpansion();
  return true;
}

bool HashVisitor::VisitInheritableAttr(const InheritableAttr *A) {
  topHash() << "Inheritable Attr";
  VisitAttr(A);
  return true;
}
/*
bool HashVisitor::VisitStmtAttr(const StmtAttr *A){
  TopHash() << "Stmt Attr";
  VisitAttr(A);
  return true;
}
*/

bool HashVisitor::VisitInheritableParamAttr(const InheritableParamAttr *A) {
  topHash() << "InheritableParamAttr";
  VisitAttr(A);
  return true;
}

/*
bool HashVisitor::VisitParameterABIAttr(const ParameterABIAttr *A){
  TopHash() << "ParameterABAttr";
  const ParameterABI pabi = getABI();
  TopHash() << pabi;//struct
  VisitAttr(A);
  return true;
}
*/

// statements
void HashVisitor::hashStmt(const Stmt *Node) {
  // Falls wir Nullpointer reinstecken
  if (!Node)
    return;

  bool Handled = mt_stmtvisitor::Visit(Node);
  if (!Handled) {
    errs() << "---- START unhandled statement ----\n";
    Node->dump();
    errs() << "----- END unhandled statement -----\n";
  }
}

bool HashVisitor::VisitStmt(const Stmt *Node) {
  errs() << "Statement Not Handled\n";
  return false;
}

bool HashVisitor::VisitCompoundStmt(const CompoundStmt *Node) {
  topHash() << "compound";
  for (CompoundStmt::const_body_iterator I = Node->body_begin(),
                                         E = Node->body_end();
       I != E; ++I) {
    hashStmt(*I);
  }
  return true;
}

bool HashVisitor::VisitBreakStmt(const BreakStmt *Node) {
  topHash() << "break";
  return true;
}

bool HashVisitor::VisitContinueStmt(const ContinueStmt *Node) {
  topHash() << AstElementContinueStmt;
  return true;
}

bool HashVisitor::VisitGotoStmt(const GotoStmt *Node) {
  topHash() << AstElementGotoStmt;
  hashDecl(Node->getLabel());
  return true;
}

bool HashVisitor::VisitLabelStmt(const LabelStmt *Node) {
  topHash() << AstElementLabelStmt;
  hashDecl(Node->getDecl());
  hashStmt(Node->getSubStmt());
  return true;
}

bool HashVisitor::VisitDoStmt(const DoStmt *Node) {
  topHash() << AstElementDoStmt;
  hashStmt(Node->getCond());
  hashStmt(Node->getBody());
  return true;
}

bool HashVisitor::VisitForStmt(const ForStmt *Node) {
  topHash() << AstElementForStmt;
  hashStmt(Node->getInit());
  hashStmt(Node->getConditionVariableDeclStmt());
  hashStmt(Node->getCond());
  hashStmt(Node->getInc());
  hashStmt(Node->getBody());
  return true;
}

bool HashVisitor::VisitIfStmt(const IfStmt *Node) {
  topHash() << AstElementIfStmt;
  hashStmt(Node->getConditionVariableDeclStmt());
  hashStmt(Node->getCond());
  hashStmt(Node->getThen());
  hashStmt(Node->getElse());
  return true;
}

bool HashVisitor::VisitNullStmt(const NullStmt *Node) {
  // macht funktional keinen Unterschied...
  topHash() << AstElementNullStmt;
  return true;
}

bool HashVisitor::VisitReturnStmt(const ReturnStmt *Node) {
  topHash() << AstElementReturnStmt;
  hashStmt(Node->getRetValue());
  return true;
}

bool HashVisitor::VisitWhileStmt(const WhileStmt *Node) {
  topHash() << AstElementWhileStmt;
  hashStmt(Node->getConditionVariableDeclStmt());
  hashStmt(Node->getCond());
  hashStmt(Node->getBody());
  return true;
}

bool HashVisitor::VisitSwitchStmt(const SwitchStmt *Node) {
  topHash() << AstElementSwitchStmt;
  hashStmt(Node->getConditionVariableDeclStmt());
  hashStmt(Node->getCond());
  hashStmt(Node->getBody());
  return true;
}

bool HashVisitor::VisitCaseStmt(const CaseStmt *Node) {
  topHash() << AstElementCaseStmt;
  hashStmt(Node->getLHS());
  hashStmt(Node->getRHS());
  hashStmt(Node->getSubStmt());
  return true;
}

bool HashVisitor::VisitDefaultStmt(const DefaultStmt *Node) {
  topHash() << AstElementDefaultStmt;
  hashStmt(Node->getSubStmt());
  return true;
}

bool HashVisitor::VisitDeclStmt(const DeclStmt *Node) {
  topHash() << AstElementDeclStmt;
  for (DeclStmt::const_decl_iterator I = Node->decl_begin(),
                                     E = Node->decl_end();
       I != E; ++I) {
    hashDecl(*I);
  }
  return true;
}

bool HashVisitor::VisitGCCAsmStmt(const GCCAsmStmt *Node) {
  topHash() << "gcc asm";
  topHash() << Node->getAsmString()->getString().str();
  return true;
}

bool HashVisitor::VisitMSAsmStmt(const MSAsmStmt *Node) {
  topHash() << "MS asm";
  topHash() << Node->getAsmString().str();
  return true;
}

bool HashVisitor::VisitAttributedStmt(const AttributedStmt *Node) {
  topHash() << "AttributedStmt";
  for (const Attr *const A : Node->getAttrs()) {
    hashAttr(A);
  }

  hashStmt(Node->getSubStmt());
  return true;
}

bool HashVisitor::VisitCapturedStmt(const CapturedStmt *Node) {
  topHash() << "CaptureStmt";
  hashStmt(Node->getCapturedStmt());
  hashDecl(Node->getCapturedDecl());
  return true;
}

// not tested
bool HashVisitor::VisitSEHExceptStmt(const SEHExceptStmt *Node) {
  topHash() << "__except";
  hashStmt(Node->getFilterExpr());
  hashStmt(Node->getBlock());
  return true;
}

// not tested
bool HashVisitor::VisitSEHFinallyStmt(const SEHFinallyStmt *Node) {
  topHash() << "__finally";
  hashStmt(Node->getBlock());
  return true;
}

// not tested
bool HashVisitor::VisitSEHLeaveStmt(const SEHLeaveStmt *Node) {
  topHash() << "__leave";
  return true;
}

// not tested
bool HashVisitor::VisitSEHTryStmt(const SEHTryStmt *Node) {
  topHash() << "__try";
  hashStmt(Node->getTryBlock());
  hashStmt(Node->getHandler());
  return true;
}

bool HashVisitor::VisitIndirectGotoStmt(const IndirectGotoStmt *Node) {
  topHash() << "IndirectGotoStmt";
  hashStmt(Node->getTarget());
  if (Node->getConstantTarget()) {
    hashDecl(Node->getConstantTarget());
  }
  return true;
}

// OpenMP directives, not tested
bool
HashVisitor::VisitOMPExecutableDirective(const OMPExecutableDirective *Node) {
  errs() << "OMPExecutableDirectives are not implemented yet.\n";
  exit(1);
}
