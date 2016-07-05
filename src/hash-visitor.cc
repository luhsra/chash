#undef NDEBUG

#include "hash-visitor.h"

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
  if (!D) return;

  const Hash::Digest *const SavedDigest = getHash(D); // TODO: move to if-cond
  if (SavedDigest) {
    topHash() << *SavedDigest;
    return;
  }


  // Visit in Pre-Order
  const unsigned Depth = beforeDescent();
  const Hash* CurrentHash = nullptr;
  if (D->getDeclContext() != nullptr && isa<TranslationUnitDecl>(D->getDeclContext())) {
    CurrentHash = pushHash();
  }

  const bool Handled = mt_declvisitor::Visit(D);
  if (!Handled) {
    errs() << "---- START unhandled -----\n";
    //    D->dump();
    errs() << "---- END unhandled -----\n";
  }

  // Decls within functions are visited by the body.
  if (!isa<FunctionDecl>(*D) && hasNodes(dyn_cast<DeclContext>(D))) {
      auto DC = cast<DeclContext>(D);
      for (const auto *const Child : DC->noload_decls()) {
          if (isa<TranslationUnitDecl>(D)) {
              if (isa<TypedefDecl>(Child)
                  || isa<RecordDecl>(Child)
                  || isa<EnumDecl>(Child))
                  continue;

              // Extern variable definitions at the top-level
              if (auto var = dyn_cast<VarDecl>(Child)) {
                if (var->hasExternalStorage()) {
                  continue;
                }
              }

              if (auto func = dyn_cast<FunctionDecl>(Child)) {
                if (func->getStorageClass() == StorageClass::SC_Extern ||
                    func->getStorageClass() == StorageClass::SC_PrivateExtern ||
                    !func->isThisDeclarationADefinition()) {
                  continue;
                }
              }
          }

          hashDecl(Child);
      }
  }

  // Visit attributes of the decl
  for (const Attr *const A : D->attrs()) {
    hashAttr(A);
  }

  afterDescent(Depth);

  if (CurrentHash != nullptr) {
    // We opened a new  hash context, close it again and hash it into the parent
    const Hash::Digest CurrentDigest = popHash(CurrentHash);

    // Store hash for underlying type
    storeHash(D, CurrentDigest);
    D->dump();

    // Hash into parent
    topHash() << CurrentDigest;
  }
}

bool HashVisitor::hasNodes(const DeclContext *DC) {
  if (!DC)
    return false;

  return DC->hasExternalLexicalStorage() ||
         DC->noload_decls_begin() != DC->noload_decls_end();
}

bool HashVisitor::VisitTranslationUnitDecl(const TranslationUnitDecl *Unit) {
  const Hash *const CurrentHash = pushHash();

  // TODO:
  // FIXME Hash Compiler Options

  afterChildren([=] {
    storeHash(Unit, popHash(CurrentHash));
    TopLevelHash << *CurrentHash;
  });

  //  Unit->dump();

  return true;
}

bool HashVisitor::VisitVarDecl(const VarDecl *D) {
  haveSeen(D, D);

  topHash() << AstElementVarDecl;
  hashName(D);
  hashType(D->getType());

  topHash() << D->getStorageClass();
  topHash() << D->getTLSKind();
  topHash() << D->isModulePrivate();
  topHash() << D->isNRVOVariable();

  if (D->hasInit()) {
    topHash() << AstElementVarDecl_init; // TODO: special case required here or
                                         // just hash Expr?
    const Expr *const E = D->getInit();
    hashStmt(E); // TODO: perhaps hashStmt(D->getInit()) instead
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
void HashVisitor::hashType(const QualType &T) {
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
    // TODO: weitere qualifier?
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
      topHash() << AstElementStructureType;
      topHash() << T.getAsString();
      return;
    }
  } else if (ActualType->isUnionType()) {
    if (haveSeen(ActualType, ActualType)) {
      topHash() << AstElementUnionType;
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
  // errs() << "\n\nVisitBuiltinType, Kind = " << T->getKind() << "\n\n";//TODO:
  topHash() << T->getKind();
  assert(!T->isSugared());
  return true;
}

bool HashVisitor::VisitPointerType(const PointerType *T) {
  topHash() << AstElementPointerType;
  QualType pointee = T->getPointeeType();
  if (inRecordType > 0) {
      // This is somewhat of a hack, since we do not include pointee
      // types within structs, but their short string representation
      // (e.g., "struct foo *")
      SplitQualType T_split = pointee.split();
      topHash() << QualType::getAsString(T_split);
  } else {
      hashType(pointee);
  }
  return true;
}

bool HashVisitor::VisitArrayType(const ArrayType *T) {
  topHash() << AstElementArrayType;
  hashType(T->getElementType());
  topHash() << "["
            << "*"
            << "]";
  return true;
}

bool HashVisitor::VisitConstantArrayType(const ConstantArrayType *T) {
  topHash() << AstElementConstantArrayType;
  hashType(T->getElementType());
  topHash() << "[" << T->getSize().getZExtValue() << "]";
  return true;
}

bool HashVisitor::VisitVariableArrayType(const VariableArrayType *T) {
  topHash() << AstElementVariableArrayType;
  hashType(T->getElementType());
  topHash() << "[";
  hashStmt(T->getSizeExpr());
  topHash() << "]";
  return true;
}

bool HashVisitor::VisitTypedefType(const TypedefType *T) {
  // must not hash AstElementTypedefType here, else there will be differences
  // between variables declared with the typedef vs. with the same type

  hashType(T->desugar());
  return true;
}

bool HashVisitor::VisitComplexType(const ComplexType *T) {
  topHash() << AstElementComplexType;
  hashType(T->getElementType());
  return true;
}

bool HashVisitor::VisitAtomicType(const AtomicType *T) {
  topHash() << AstElementAtomicType;
  hashType(T->getValueType());
  return true;
}

bool HashVisitor::VisitTypeOfExprType(const TypeOfExprType *T) {
  topHash() << AstElementTypeOfExprType;
  hashType(T->desugar());
  return true;
}

bool HashVisitor::VisitTypeOfType(const TypeOfType *T) {
  topHash() << AstElementTypeOfType;
  hashType(T->desugar());
  return true;
}

bool HashVisitor::VisitParenType(const ParenType *T) {
  topHash() << AstElementParenType;
  hashType(T->desugar());
  return true;
}

bool HashVisitor::VisitFunctionType(const FunctionType *T) {
  topHash() << AstElementFunctionType;
  hashType(T->getReturnType());
  topHash() << T->getRegParmType();
  topHash() << T->getCallConv();
  return true;
}

bool HashVisitor::VisitFunctionProtoType(const FunctionProtoType *T) {
  topHash() << AstElementFunctionProtoType;
  hashType(T->getReturnType());
  for (const QualType &QT : T->getParamTypes()) {
    hashType(QT);
  }
  topHash() << T->getRegParmType();
  topHash() << T->getCallConv();
  return true;
}

bool HashVisitor::VisitEnumType(const EnumType *T) {
  topHash() << AstElementEnumType;
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

bool HashVisitor::VisitAttributedType(const AttributedType *T) {
  topHash() << AstElementAttributedType;
  topHash() << T->getAttrKind();
  hashType(T->getModifiedType());
  hashType(T->getEquivalentType());
  return true;
}

bool HashVisitor::VisitUnaryTransformType(const UnaryTransformType *T) {
  topHash() << AstElementUnaryTransformType;
  hashType(T->getBaseType());
  hashType(T->getUnderlyingType());
  topHash() << T->getUTTKind();
  return true;
}

bool HashVisitor::VisitDecayedType(const DecayedType *T) {
  topHash() << AstElementDecayedType;
  hashType(T->getOriginalType());
  hashType(T->getAdjustedType());
  hashType(T->getPointeeType());
  return true;
}

bool HashVisitor::VisitAdjustedType(const AdjustedType *T) {
  topHash() << AstElementAdjustedType;
  hashType(T->getOriginalType());
  hashType(T->getAdjustedType());
  return true;
}

bool HashVisitor::VisitElaboratedType(const ElaboratedType *T) {
  topHash() << AstElementElaboratedType;
  // FIXME: Hash NestedNameSpecifier.
  hashType(T->getNamedType());
  topHash() << T->getKeyword();
  return true;
}


bool HashVisitor::VisitRecordType(const RecordType *RT) {
  if (const Hash::Digest *const D = getHash(RT)) {
    topHash() << *D;
    return true;
  }

  if (RT->isStructureType()) {
      topHash() << AstElementStructureType;
  } else if (RT->isUnionType()) {
      topHash() << AstElementUnionType;
  } else {
      assert(false && "Neither Union nor Struct");
      return false;
  }

  const RecordDecl *const RD = RT->getDecl();

  inRecordType++; // increase and decrease inRecordType level
  hashDecl(RD);
  inRecordType--;


  return true;
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
  // DeclRefExpr = usage of variables ("expressions which refer to variable
  // declarations")
  const ValueDecl *const ValDecl = Node->getDecl();
  topHash() << AstElementDeclRefExpr;
  // TODO:
  // FIXME:	spaeter Auskommentiertes(isa-Zeug) einkommentieren (oder Problem
  // anders beheben)
  //		Andere Decls mehrfach referenzieren => Problem
  //		evtl. auch Zyklus in anderer Decl
  if (/*(isa<VarDecl>(Decl) || isa<FunctionDecl>(Decl)) && */
      haveSeen(ValDecl, ValDecl)) {
    if (const Hash::Digest *const D = getHash(ValDecl)) {
      topHash() << *D;
    } else {
      // if(!isa<VarDecl>(Decl) && !isa<FunctionDecl>(Decl)){
      //	errs() << "Not a VarDecl or FunctionDecl:\n";
      //	Decl->dump();
      //}
      assert(isa<VarDecl>(ValDecl) || isa<FunctionDecl>(ValDecl));
      if (isa<VarDecl>(ValDecl)) {
        topHash() << "VarDeclDummy"; // TODO: ?
        const VarDecl *const VD = static_cast<const VarDecl *>(ValDecl);
        dummyVarDecl(VD); // TODO: warum dummy???
      } else {
        const FunctionDecl *const FD =
            static_cast<const FunctionDecl *>(ValDecl);
        dummyFunctionDecl(FD); // TODO: warum dummy???
      }
    }
  } else {
    hashDecl(ValDecl);
  }
  hashName(Node->getFoundDecl());
  return true;
}

bool HashVisitor::VisitPredefinedExpr(const PredefinedExpr *Node) {
  topHash() << AstElementPredefinedExpr;
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
  const fltSemantics *const semantics = &Node->getValue().getSemantics();
  if (semantics == (const fltSemantics *)&APFloat::IEEEdouble) {
    const double Value = Node->getValue().convertToDouble();
    topHash() << *reinterpret_cast<const uint64_t *>(&Value);
  } else if (semantics == (const fltSemantics *)&APFloat::IEEEsingle) {
    const double Value = Node->getValue().convertToFloat();
    topHash() << *reinterpret_cast<const uint64_t *>(&Value);
  } else if (semantics == &APFloat::IEEEhalf ||
             semantics == &APFloat::x87DoubleExtended ||
             semantics == &APFloat::IEEEquad ||
             semantics == &APFloat::PPCDoubleDouble) {
    SmallVector<char, 30> vector;
    Node->getValue().toString(vector);
    std::string str;
    str.reserve(vector.size());
    for (const char c : vector) {
      str += c;
    }
    topHash() << str;
  } else {
    assert(0 && "unknown FloatingLiteral");
  }
  return true;
}

bool HashVisitor::VisitStringLiteral(const StringLiteral *Node) {
  topHash() << AstElementStringLiteral;
  hashType(Node->getType());
  topHash() << Node->getBytes().str();
  return true;
}

bool HashVisitor::VisitInitListExpr(const InitListExpr *Node) {
  topHash() << AstElementInitListExpr;
  for (unsigned I = 0, E = Node->getNumInits(); I < E; ++I) {
    hashStmt(Node->getInit(I));
  }
  if (Node->hasArrayFiller()) {
    hashStmt(Node->getArrayFiller());
  }
  return true;
}

bool HashVisitor::VisitUnaryOperator(const UnaryOperator *Node) {
  topHash() << AstElementUnaryOperator;
  hashStmt(Node->getSubExpr());
  topHash() << Node->getOpcode();
  hashType(Node->getType());
  return true;
}

bool HashVisitor::VisitUnaryExprOrTypeTraitExpr(
    const UnaryExprOrTypeTraitExpr *Node) {
  topHash() << AstElementUnaryExprOrTypeTraitExpr;
  topHash() << Node->getKind();
  if (Node->isArgumentType()) {
    hashType(Node->getArgumentType());
  } else {
    hashStmt(Node->getArgumentExpr());
  }
  return true;
}

bool HashVisitor::VisitMemberExpr(const MemberExpr *Node) {
  topHash() << "member"; // TODO: replace with constant
  const Expr *const Base = Node->getBase();
  hashStmt(Base);

  const ValueDecl *const MemberDecl = Node->getMemberDecl();
  hashDecl(MemberDecl);

  topHash() << Node->isArrow();
  return true;
}

bool HashVisitor::VisitBinaryOperator(const BinaryOperator *Node) {
  topHash() << AstElementBinaryOperator;
  hashStmt(Node->getLHS());
  hashStmt(Node->getRHS());
  topHash() << Node->getOpcode();
  hashType(Node->getType());
  return true;
}

// erbt von BinaryOperator
bool
HashVisitor::VisitCompoundAssignOperator(const CompoundAssignOperator *Node) {
  topHash() << AstElementCompoundAssignmentOperator;
  hashStmt(Node->getLHS());
  hashStmt(Node->getRHS());
  topHash() << Node->getOpcode();
  hashType(Node->getType());
  return true;
}

bool HashVisitor::VisitAddrLabelExpr(const AddrLabelExpr *Node) {
  topHash() << AstElementAddrLabelExpr;
  hashStmt(Node->getLabel()->getStmt());
  return true;
}

bool HashVisitor::VisitImaginaryLiteral(const ImaginaryLiteral *Node) {
  topHash() << AstElementImaginaryLiteral;
  hashType(Node->getType());
  hashStmt(Node->getSubExpr());
  return true;
}

bool HashVisitor::VisitCompoundLiteralExpr(const CompoundLiteralExpr *Node) {
  topHash() << AstElementCompoundLiteralExpr;
  hashType(Node->getType());
  hashStmt(Node->getInitializer());
  return true;
}

bool HashVisitor::VisitAbstractConditionalOperator(
    const AbstractConditionalOperator *Node) {
  topHash() << AstElementAbstractConditionalOperator;
  hashType(Node->getType());
  hashStmt(Node->getCond());
  hashStmt(Node->getTrueExpr());
  hashStmt(Node->getFalseExpr());
  return true;
}

bool HashVisitor::VisitBinaryConditionalOperator(
    const BinaryConditionalOperator *Node) {
  topHash() << AstElementBinaryConditionalOperator;
  hashType(Node->getType());
  hashStmt(Node->getCond());
  hashStmt(Node->getCommon());
  hashStmt(Node->getTrueExpr());
  hashStmt(Node->getFalseExpr());
  return true;
}

bool HashVisitor::VisitCallExpr(const CallExpr *Node) {
  topHash() << AstElementCallExpr;
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
  topHash() << AstElementOffsetOfExpr;
  hashType(Node->getType());
  for (unsigned I = 0, E = Node->getNumExpressions(); I < E; ++I) {
    hashStmt(Node->getIndexExpr(I));
  }
  for (unsigned I = 0, E = Node->getNumComponents(); I < E; ++I) {
    const OffsetOfNode Offset = Node->getComponent(I);
    topHash() << AstElementOffsetOfNode;
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
  topHash() << AstElementParenExpr;
  hashType(Node->getType());
  hashStmt(Node->getSubExpr());
  return true;
}

bool HashVisitor::VisitAtomicExpr(const AtomicExpr *Node) {
  AtomicExpr *const MutableNode = const_cast<AtomicExpr *>(Node);

  topHash() << AstElementAtomicExpr;
  hashType(Node->getType());
  Expr **SubExprs = MutableNode->getSubExprs();
  for (unsigned I = 0, E = MutableNode->getNumSubExprs(); I < E; ++I) {
    hashStmt(SubExprs[I]);
  }
  return true;
}

bool HashVisitor::VisitParenListExpr(const ParenListExpr *Node) {
  topHash() << AstElementParenListExpr;
  hashType(Node->getType());
  for (const Expr *const E : const_cast<ParenListExpr *>(Node)->exprs()) {
    hashStmt(E); // TODO: expr instead of stmt: ok or cast explicitly?
  }
  return true;
}

bool HashVisitor::VisitDesignatedInitExpr(const DesignatedInitExpr *Node) {
  DesignatedInitExpr *const MutableNode =
      const_cast<DesignatedInitExpr *>(Node);

  topHash() << AstElementDesignatedInitExpr;
  hashType(Node->getType());
  for (unsigned I = 0, E = MutableNode->getNumSubExprs(); I < E; ++I) {
    hashStmt(MutableNode->getSubExpr(I));
  }
  for (unsigned I = 0, E = MutableNode->size(); I < E; ++I) {
    const DesignatedInitExpr::Designator *const Des =
        MutableNode->getDesignator(I);
    topHash() << AstElementDesignator;
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
  topHash() << AstElementArraySubscriptExpr;
  hashStmt(Node->getLHS());
  hashStmt(Node->getRHS());
  hashStmt(Node->getBase());
  hashStmt(Node->getIdx());
  return true;
}

bool
HashVisitor::VisitImplicitValueInitExpr(const ImplicitValueInitExpr *Node) {
  topHash() << AstElementImplicitValueInitExpr;
  hashType(Node->getType());
  return true;
}

bool HashVisitor::VisitVAArgExpr(const VAArgExpr *Node) {
  topHash() << AstElementVAArgExpr;
  hashType(Node->getType());
  hashStmt(Node->getSubExpr());
  topHash() << Node->isMicrosoftABI();
  return true;
}

bool HashVisitor::VisitBlockExpr(const BlockExpr *Node) {
  topHash() << AstElementBlockExpr;
  hashDecl(Node->getBlockDecl());
  hashStmt(Node->getBody());
  return true;
}

// common Decls

bool HashVisitor::VisitBlockDecl(const BlockDecl *D) {
  topHash() << AstElementBlockDecl;
  for (const ParmVarDecl *const PVD : D->parameters()) {
    VisitVarDecl(PVD);
  }
  hashStmt(D->getBody());
  return true;
}

bool HashVisitor::VisitFunctionDecl(const FunctionDecl *D) {
  haveSeen(D, D);

  topHash() << AstElementFunctionDecl;
  topHash() << D->getNameInfo().getName().getAsString();
  hashStmt(D->getBody());
  topHash() << D->isDefined();
  topHash() << D->isThisDeclarationADefinition();
  topHash() << D->isVariadic();
  topHash() << D->isVirtualAsWritten();
  topHash() << D->isPure();
  topHash() << D->hasImplicitReturnZero();
  topHash() << D->hasPrototype();
  topHash() << D->hasWrittenPrototype();
  topHash() << D->hasInheritedPrototype();
  topHash() << D->isMain();
  topHash() << D->isExternC();
  topHash() << D->isGlobal();
  topHash() << D->isNoReturn();
  topHash() << D->hasSkippedBody(); //???
  topHash() << D->getBuiltinID();

  topHash() << D->getStorageClass(); // static and stuff
  topHash() << D->isInlineSpecified();
  topHash() << D->isInlined();

  // hash all parameters
  for (const ParmVarDecl *const PVD : D->parameters()) {
    hashDecl(PVD);
  }

  // vielleicht will man das ja auch:
  for (const NamedDecl *const ND : D->getDeclsInPrototypeScope()) {
    hashDecl(ND);
  }

  // visit QualType
  hashType(D->getReturnType());

  // here an error (propably nullptr) occured
  // TODO: vllt. besser mit assertions checken (s.o.)
  // TODO: assertions crashen! unbedingt fixen!
  if (const IdentifierInfo *const IdentInfo = D->getLiteralIdentifier()) {
    if (const char *const IdentName = IdentInfo->getNameStart()) {
      topHash() << IdentName;
    }
  }
  // TODO: replace the if-code above with this and fix problem!
  //  const IdentifierInfo *const IdentInfo = Node->getLiteralIdentifier();
  //  assert(IdentInfo != nullptr);
  //  const char *const IdentName = IdentInfo->getNameStart();
  //  assert(IdentName != nullptr);
  //  topHash() << IdentName;

  topHash() << D->getMemoryFunctionKind(); // maybe needed

  return true;
}

bool HashVisitor::VisitLabelDecl(const LabelDecl *D) {
  topHash() << AstElementLabelDecl;
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
  topHash() << AstElementEnumDecl;
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
  topHash() << AstElementEnumConstantDecl;
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
  topHash() << AstElementIndirectFieldDecl;
  for (IndirectFieldDecl::chain_iterator I = D->chain_begin(),
                                         E = D->chain_end();
       I != E; ++I) {
    hashDecl(*I);
  }
  VisitValueDecl(D);
  return true;
}

bool HashVisitor::VisitFieldDecl(const FieldDecl *FD) {
    topHash() << AstElementFieldDecl;
    VisitValueDecl(FD); // Call Super Function

    if (FD->isBitField()) {
        hashStmt(FD->getBitWidth());
    }
    return true;
}

// called by children
bool HashVisitor::VisitValueDecl(const ValueDecl *D) {
  hashType(D->getType());
  hashName(D);
  return true;
}

bool HashVisitor::VisitFileScopeAsmDecl(const FileScopeAsmDecl *D) {
  topHash() << AstElementFileScopeAsmDecl;
  if (const StringLiteral *const SL = D->getAsmString()) {
    hashStmt(SL);
  }
  return true;
}

bool HashVisitor::VisitCapturedDecl(const CapturedDecl *D) {
  topHash() << AstElementCapturedDecl;
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
  topHash() << AstElementAttr;
  topHash() << A->getKind(); // hash enum
  topHash() << A->isPackExpansion();
  return true;
}

bool HashVisitor::VisitInheritableAttr(const InheritableAttr *A) {
  topHash() << AstElementInheritableAttr;
  VisitAttr(A);
  return true;
}
/*
bool HashVisitor::VisitStmtAttr(const StmtAttr *A){
  TopHash() << AstElementStmtAttr; //TODO: does not exist yet
  VisitAttr(A);
  return true;
}
*/

bool HashVisitor::VisitInheritableParamAttr(const InheritableParamAttr *A) {
  topHash() << AstElementInheritableParamAttr;
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
    //    Node->dump();
    errs() << "----- END unhandled statement -----\n";
  }
}

bool HashVisitor::VisitStmt(const Stmt *Node) {
  errs() << "Statement Not Handled\n";
  return false;
}

bool HashVisitor::VisitCompoundStmt(const CompoundStmt *Node) {
  topHash() << AstElementCompoundStmt;
  for (CompoundStmt::const_body_iterator I = Node->body_begin(),
                                         E = Node->body_end();
       I != E; ++I) {
    hashStmt(*I);
  }
  return true;
}

bool HashVisitor::VisitBreakStmt(const BreakStmt *Node) {
  topHash() << AstElementBreakStmt;
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
  topHash() << AstElementGCCAsmStmt;
  topHash() << Node->getAsmString()->getString().str();
  return true;
}

bool HashVisitor::VisitMSAsmStmt(const MSAsmStmt *Node) {
  topHash() << AstElementMSAsmStmt;
  topHash() << Node->getAsmString().str();
  return true;
}

bool HashVisitor::VisitAttributedStmt(const AttributedStmt *Node) {
  topHash() << AstElementAttributedStmt;
  for (const Attr *const A : Node->getAttrs()) {
    hashAttr(A);
  }

  hashStmt(Node->getSubStmt());
  return true;
}

bool HashVisitor::VisitCapturedStmt(const CapturedStmt *Node) {
  topHash() << AstElementCaptureStmt;
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
  topHash() << AstElementIndirectGotoStmt;
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
