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

/// VisitXyz-Methods return true if case was handled
class TranslationUnitHashVisitor
    : public ConstDeclVisitor<TranslationUnitHashVisitor, bool>,
      public ConstStmtVisitor<TranslationUnitHashVisitor, bool>,
      public TypeVisitor<TranslationUnitHashVisitor, bool> {

  typedef ConstDeclVisitor<TranslationUnitHashVisitor, bool> mt_declvisitor;
  typedef ConstStmtVisitor<TranslationUnitHashVisitor, bool> mt_stmtvisitor;
  typedef TypeVisitor<TranslationUnitHashVisitor, bool> mt_typevisitor;

  Hash TopLevelHash;

  // /// Pending[i] is an action to hash an entity at level i.
  bool FirstChild;
  llvm::SmallVector<std::function<void()>, 32> Pending;

  /// Hash a child of the current node.
  unsigned beforeDescent() {
    FirstChild = true;
    return Pending.size();
  }

  template <typename Fn> void afterChildren(Fn Func) {
    if (FirstChild) {
      Pending.push_back(std::move(Func));
    } else {
      Pending.back()();
      Pending.back() = std::move(Func);
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
  // In this storage we save hashes for various memory objects
  std::map<const Type *, Hash::Digest> TypeSilo;
  std::map<const Decl *, Hash::Digest> DeclSilo;

  // Utilities
  bool hasNodes(const DeclContext *DC);

  void hashDecl(const Decl *D);
  void hashStmt(const Stmt *Node);
  void hashType(const QualType &T);
  void hashAttr(const Attr *A);

  void hashName(const NamedDecl *ND);

  // C Declarations
  bool VisitTranslationUnitDecl(const TranslationUnitDecl *Unit);
  bool VisitVarDecl(const VarDecl *D);
  /// Not interesting
  bool VisitTypedefDecl(const TypedefDecl *) { return true; }

  bool VisitRecordDecl(const RecordDecl *D) { return true; }
  bool VisitFieldDecl(const FieldDecl *D);

  // C Types
  bool VisitBuiltinType(const BuiltinType *T);
  bool VisitPointerType(const PointerType *T);
  bool VisitArrayType(const ArrayType *T);
  bool VisitConstantArrayType(const ConstantArrayType *T);
  bool VisitVariableArrayType(const VariableArrayType *T);
  bool VisitRecordType(const RecordType *);
  bool VisitTypedefType(const TypedefType *T);
  bool VisitComplexType(const ComplexType *T);
  bool VisitAtomicType(const AtomicType *T);
  bool VisitTypeOfExprType(const TypeOfExprType *T);
  bool VisitTypeOfType(const TypeOfType *T);
  bool VisitParenType(const ParenType *T);
  bool VisitFunctionType(const FunctionType *T);
  bool VisitFunctionProtoType(const FunctionProtoType *T);
  bool VisitEnumType(const EnumType *T);
  bool VisitAttributedType(const AttributedType *T);
  bool VisitUnaryTransformType(const UnaryTransformType *T);
  bool VisitDecayedType(const DecayedType *T);
  bool VisitAdjustedType(const AdjustedType *T);
  bool VisitElaboratedType(const ElaboratedType *T);

  std::string getHash(unsigned *ProcessedBytes = nullptr);

  // C Exprs (no clang-builtins, ...)
  /*bool VisitExpr(const Expr *Node);*/
  bool VisitCastExpr(const CastExpr *Node);
  bool VisitDeclRefExpr(const DeclRefExpr *Node);
  bool VisitPredefinedExpr(const PredefinedExpr *Node);
  bool VisitCharacterLiteral(const CharacterLiteral *Node);
  bool VisitIntegerLiteral(const IntegerLiteral *Node);
  bool VisitFloatingLiteral(const FloatingLiteral *Node);
  bool VisitStringLiteral(const StringLiteral *Node);
  bool VisitInitListExpr(const InitListExpr *Node);
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
  bool
  VisitAbstractConditionalOperator(const AbstractConditionalOperator *Node);
  bool VisitBinaryConditionalOperator(const BinaryConditionalOperator *Node);
  bool VisitCallExpr(const CallExpr *Node);
  bool VisitOffsetOfExpr(const OffsetOfExpr *Node);
  bool VisitParenExpr(const ParenExpr *Node);
  bool VisitAtomicExpr(const AtomicExpr *Node);
  bool VisitParenListExpr(const ParenListExpr *Node);
  bool VisitDesignatedInitExpr(const DesignatedInitExpr *Node);
  bool VisitStmtExpr(const StmtExpr *Node);
  bool VisitVAArgExpr(const VAArgExpr *Node);

  // TODO: evtl. ImplicitValueInitExpr, GenericSelectionExpr, ArraySubscriptExpr
  // TODO: evtl. OpaqueValueExpr, ExtVectorElementExpr (Beschreibung klingt nach
  // C++)

  // declarations
  bool VisitFunctionDecl(const FunctionDecl *D);
  bool VisitBlockDecl(const BlockDecl *D);
  bool VisitLabelDecl(const LabelDecl *D);
  bool VisitEnumDecl(const EnumDecl *D);
  bool VisitEnumConstantDecl(const EnumConstantDecl *D);
  bool VisitImplicitParamDecl(const ImplicitParamDecl *D);
  bool VisitParmVarDecl(const ParmVarDecl *D);
  // DeclaratorDecl done...
  bool VisitIndirectFieldDecl(const IndirectFieldDecl *D);
  bool VisitFieldDecl(const IndirectFieldDecl *D);
  bool VisitValueDecl(const ValueDecl *D); // maybe called by children
  bool VisitFileScopeAsmDecl(const FileScopeAsmDecl *D);
  bool VisitCapturedDecl(const CapturedDecl *D);

  // Attrs
  // uncommented Attr not found in namespace
  bool VisitAttr(const Attr *A);
  bool VisitInheritableAttr(const InheritableAttr *A);
  // bool VisitStmtAttr(const StmtAttr *A);
  bool VisitInheritableParamAttr(const InheritableParamAttr *A);
  // bool VisitParameterABIAttr(const ParameterABIAttr *A);

  // statements
  bool VisitStmt(const Stmt *Node);
  bool VisitCompoundStmt(const CompoundStmt *Node);
  bool VisitBreakStmt(const BreakStmt *Node);
  bool VisitContinueStmt(const ContinueStmt *Node);
  bool VisitGotoStmt(const GotoStmt *Node);
  bool VisitLabelStmt(const LabelStmt *Node);
  bool VisitDoStmt(const DoStmt *Node);
  bool VisitForStmt(const ForStmt *Node);
  bool VisitIfStmt(const IfStmt *Node);
  bool VisitNullStmt(const NullStmt *Node);
  bool VisitReturnStmt(const ReturnStmt *Node);
  bool VisitWhileStmt(const WhileStmt *Node);
  bool VisitSwitchStmt(const SwitchStmt *Node);
  bool VisitCaseStmt(const CaseStmt *Node);
  bool VisitDefaultStmt(const DefaultStmt *Node);
  bool VisitDeclStmt(const DeclStmt *Node);
  bool VisitGCCAsmStmt(const GCCAsmStmt *Node);
  bool VisitMSAsmStmt(const MSAsmStmt *Node);

  // not sure if we need this
  bool VisitAttributedStmt(const AttributedStmt *Node);
  bool VisitCapturedStmt(const CapturedStmt *Node);
  bool VisitSEHExceptStmt(const SEHExceptStmt *Node);
  bool VisitSEHFinallyStmt(const SEHFinallyStmt *Node);
  bool VisitSEHLeaveStmt(const SEHLeaveStmt *Node);
  bool VisitSEHTryStmt(const SEHTryStmt *Node);
  bool VisitIndirectGotoStmt(const IndirectGotoStmt *Node);

  // not implemented
  bool VisitOMPExecutableDirective(const OMPExecutableDirective *Node);

protected:
  enum AstElementPrefix {
    AstElementVarDecl = 0xb19c2ee2,
    AstElementVarDecl_init = 0x66734486,
    AstElementImplicitParamDecl = 0xd04f138f,
    AstElementParamVarDecl = 0x1fe2fcb9,

    AstElementStmtExpr = 0xf4bb377e,
    AstElementCastExpr = 0x7c505e88,

    AstElementCharacterLiteral = 0x2a1c033f,
    AstElementIntegerLiteral = 0x7b2daa87,
    AstElementFloatingLiteral = 0xceee8473,
    AstElementStringLiteral = 0xe5846c45,

    AstElementForStmt = 0xec4e334f,
    AstElementIfStmt = 0x3de06c3c,
    AstElementNullStmt = 0x777400e0,
    AstElementDoStmt = 0xa80405bd,
    AstElementGotoStmt = 0xec2a6be8,
    AstElementContinueStmt = 0x2c518360,
    AstElementReturnStmt = 0x1cf8354e,
    AstElementWhileStmt = 0x6cb85f96,
    AstElementLabelStmt = 0xe3d17613,
    AstElementSwitchStmt = 0x6ef423db,
    AstElementCaseStmt = 0x9640cc21,
    AstElementDefaultStmt = 0x2f6febe9,
    AstElementDeclStmt = 0xbe748556,

    AstElementPointerType = 0x5b868718,
    AstElementArrayType = 0xd0b37bef,
    AstElementConstantArrayType = 0x6439c9ef,
    AstElementVariableArrayType = 0x74887cd4,
    AstElementComplexType = 0x75d5304a,
    AstElementAtomicType = 0x8a024d89,
    AstElementTypeOfExprType = 0x3417cfda,
    AstElementTypeOfType = 0x98090139,
    AstElementParenType = 0x7c2df2fc,
    AstElementFunctionType = 0x8647819b,
    AstElementFunctionProtoType = 0x4dd5f204,
    AstElementEnumType = 0x4acd4cde,
    AstElementTagType = 0x94c7a399,
    AstElementAttributedType = 0xddc8426,
    AstElementUnaryTransformType = 0xca8afa5b,
    AstElementDecayedType = 0x707c703e,
    AstElementAdjustedType = 0x9936193,
    AstElementElaboratedType = 0x96681107,

    AstElementDeclRefExpr = 0xa33a24f3,
    AstElementPredefinedExpr = 0xffb3cc20,
    AstElementInitListExpr = 0xe23aaddd,
    AstElementUnaryExprOrTypeTraitExpr = 0xb4995380,
    AstElementMemberExpr = 0xe682fc67,
    AstElementAddrLabelExpr = 0xe511b92e,
    AstElementCompoundLiteralExpr = 0xc54ffefa,
    AstElementCallExpr = 0x427cc6e8,
    AstElementOffsetOfExpr = 0x48232f36,
    AstElementParenExpr = 0xf1a9c911,
    AstElementAtomicExpr = 0x7e5497b7,
    AstElementParenListExpr = 0x64600f,
    AstElementDesignatedInitExpr = 0x8d017154,
    AstElementDesignator = 0x6cf40f99, // TODO: vllt. woanders "einsortieren"?
    AstElementArraySubscriptExpr = 0x8c7ab6b2, // TODO: = ArrayAccess?
    AstElementImplicitValueInitExpr = 0xfe7647fa,
    AstElementVAArgExpr = 0xdf10fedc,
    AstElementBlockExpr = 0xcc75aacd,

    AstElementBlockDecl = 0x761e230f,
    AstElementFunctionDecl = 0x2a34b689,
    AstElementLabelDecl = 0xff6db781,
    AstElementEnumDecl = 0xc564aed1,
    AstElementEnumConstantDecl = 0x11050d85,
    AstElementIndirectFieldDecl = 0x937408ea,
    AstElementValueDecl = 0xbb06d011,
    AstElementFileScopeAsmDecl = 0x381879fa,
    AstElementCapturedDecl = 0xa3a884ed,

    AstElementAttr = 0x56b6cba9,
    AstElementInheritableAttr = 0x7c0b04ce,
    AstElementInheritableParamAttr = 0x6a4fdb90,

    AstElementCompoundStmt = 0x906b6fb4,
    AstElementBreakStmt = 0x530ae0a9,
    AstElementGCCAsmStmt = 0x652782d6,
    AstElementMSAsmStmt = 0xccd123ef,
    AstElementAttributedStmt = 0x8e36d148,
    AstElementCaptureStmt = 0x1cafe3db,
    AstElementIndirectGotoStmt = 0x98888356,

    AstElementStructureType = 0xa5b0d36d,
    AstElementUnionType = 0x5057c896,

    AstElementUnaryOperator = 0x496a1fb5,
    AstElementBinaryOperator = 0xa6339d46,
    AstElementCompoundAssignmentOperator = 0x9c582bf3,
    AstElementAbstractConditionalOperator = 0x151982b7,
    AstElementBinaryConditionalOperator = 0x40d2aa93,

    // TODO: sort these:
    AstElementImaginaryLiteral = 0xe340180e,
    AstElementOffsetOfNode = 0x17f2d532,
    AstElementFieldDecl = 0xac0c83d4 // TODO!

    // AstElement = 0x27892cea
    // AstElement = 0x4ed393c3
    // AstElement = 0x2e2321ad
    // AstElement = 0xfe447195
    // AstElement = 0xe9bda7ae
    // AstElement = 0x366466fc
    // AstElement = 0xd6b02f4e
    // AstElement = 0x9b31edf6
    // AstElement = 0x45b2a746
    // AstElement = 0xc853e2ac
    // AstElement = 0xbcfa92ec
    // AstElement = 0x1cc7935
  };

  int inRecordType = 0; // Flag used to not follow pointers within structs

  std::set<const void *> SeenTypes;
  bool haveSeen(const void *Key) {
    return SeenTypes.find(Key) != SeenTypes.end();
  }

  bool dummyFunctionDecl(const FunctionDecl *FD) {
    // Ignore extern declarations
    if (FD->getStorageClass() == StorageClass::SC_Extern ||
        FD->getStorageClass() == StorageClass::SC_PrivateExtern) {
      return true;
    }

    Hash() << "FunctionDecl"; // TODO: replace with constant?
    Hash() << FD->getNameInfo().getName().getAsString();

    Hash() << FD->isDefined();
    Hash() << FD->isThisDeclarationADefinition();
    Hash() << FD->isVariadic();
    Hash() << FD->isVirtualAsWritten();
    Hash() << FD->isPure();
    Hash() << FD->hasImplicitReturnZero();
    Hash() << FD->hasPrototype();
    Hash() << FD->hasWrittenPrototype();
    Hash() << FD->hasInheritedPrototype();
    Hash() << FD->isMain();
    Hash() << FD->isExternC();
    Hash() << FD->isGlobal();
    Hash() << FD->isNoReturn();
    Hash() << FD->hasSkippedBody(); //???
    Hash() << FD->getBuiltinID();

    Hash() << FD->getStorageClass(); // static and stuff
    Hash() << FD->isInlineSpecified();
    Hash() << FD->isInlined();

    // hash all parameters
    for (ParmVarDecl *PVD : FD->parameters()) {
      hashDecl(PVD);
    }

    // vielleicht will man das ja auch:
    for (NamedDecl *ND : FD->getDeclsInPrototypeScope()) {
      hashDecl(ND);
    }

    // visit QualType
    hashType(FD->getReturnType());

    // here an error (propably nullptr) occured
    // TODO: fix this!
    if (const IdentifierInfo *const IdentInfo = FD->getLiteralIdentifier()) {
      if (const char *const IdentName = IdentInfo->getNameStart()) {
        Hash() << IdentName;
      }
    }

    Hash() << FD->getMemoryFunctionKind(); // maybe needed
    return true;
  }

  void dummyVarDecl(const VarDecl *VD) {
    hashName(VD);
    hashType(VD->getType());
    Hash() << VD->getStorageClass();
    Hash() << VD->getTLSKind();
    Hash() << VD->isModulePrivate();
    Hash() << VD->isNRVOVariable();
  }

  // Hash Silo
  void storeHash(const Type *Obj, Hash::Digest Dig) { TypeSilo[Obj] = Dig; }
  void storeHash(const Decl *Obj, Hash::Digest Dig) { DeclSilo[Obj] = Dig; }

  const Hash::Digest *getHash(const Type *Obj) {
    if (TypeSilo.find(Obj) != TypeSilo.end()) {
      return &TypeSilo[Obj];
    }
    return nullptr;
  }
  const Hash::Digest *getHash(const Decl *Obj) {
    if (DeclSilo.find(Obj) != DeclSilo.end()) {
      return &DeclSilo[Obj];
    }
    return nullptr;
  }

  Hash *pushHash() {
    HashStack.push_back(Hash());

    //	llvm::errs() << "  PushHash mit Groesse: " << HashStack.size() << " und
    // Rueckgabewert: " << (&HashStack.back()) << "\n";

    return &HashStack.back();
  }

  Hash::Digest popHash(const Hash *ShouldBe = nullptr) {

    // llvm::errs() << "  PopHash mit Groesse: " << HashStack.size() << " und
    // Parameter: " << (should_be) << "\n";
    // if(should_be != &HashStack.back()){
    // llvm::errs() << "	but Stack-Level is: " << &HashStack.back() <<
    // "\n";
    //}

    assert(!ShouldBe || ShouldBe == &HashStack.back());

    // Finalize the Hash
    Hash::Digest CurrentDigest = topHash().getDigest();
    HashStack.pop_back();
    return CurrentDigest;
  }

  Hash &topHash() { return HashStack.back(); }
};

#endif
