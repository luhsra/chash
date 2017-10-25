#ifndef __HASH_VISITOR
#define __HASH_VISITOR

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <string>
#include <set>
#include <map>
#include <vector>
#include <tuple>

#include "Hash.h"

using namespace clang;

namespace TranslationUnitHashVisitorPrefix {
enum  {
    VarDecl = 0xb19c2ee2,
    VarDecl_init = 0x66734486,
    ImplicitParamDecl = 0xd04f138f,
    ParamVarDecl = 0x1fe2fcb9,

    StmtExpr = 0xf4bb377e,
    CastExpr = 0x7c505e88,

    CharacterLiteral = 0x2a1c033f,
    IntegerLiteral = 0x7b2daa87,
    FloatingLiteral = 0xceee8473,
    StringLiteral = 0xe5846c45,

    ForStmt = 0xec4e334f,
    IfStmt = 0x3de06c3c,
    NullStmt = 0x777400e0,
    DoStmt = 0xa80405bd,
    GotoStmt = 0xec2a6be8,
    ContinueStmt = 0x2c518360,
    ReturnStmt = 0x1cf8354e,
    WhileStmt = 0x6cb85f96,
    LabelStmt = 0xe3d17613,
    SwitchStmt = 0x6ef423db,
    CaseStmt = 0x9640cc21,
    DefaultStmt = 0x2f6febe9,
    DeclStmt = 0xbe748556,

    PointerType = 0x5b868718,
    ArrayType = 0xd0b37bef,
    ConstantArrayType = 0x6439c9ef,
    VariableArrayType = 0x74887cd4,
    ComplexType = 0x75d5304a,
    AtomicType = 0x8a024d89,
    TypeOfExprType = 0x3417cfda,
    TypeOfType = 0x98090139,
    ParenType = 0x7c2df2fc,
    FunctionType = 0x8647819b,
    FunctionProtoType = 0x4dd5f204,
    EnumType = 0x4acd4cde,
    TagType = 0x94c7a399,
    AttributedType = 0xddc8426,
    UnaryTransformType = 0xca8afa5b,
    DecayedType = 0x707c703e,
    AdjustedType = 0x9936193,
    ElaboratedType = 0x96681107,

    DeclRefExpr = 0xa33a24f3,
    PredefinedExpr = 0xffb3cc20,
    InitListExpr = 0xe23aaddd,
    UnaryExprOrTypeTraitExpr = 0xb4995380,
    MemberExpr = 0xe682fc67,
    AddrLabelExpr = 0xe511b92e,
    CompoundLiteralExpr = 0xc54ffefa,
    CallExpr = 0x427cc6e8,
    OffsetOfExpr = 0x48232f36,
    ParenExpr = 0xf1a9c911,
    AtomicExpr = 0x7e5497b7,
    ParenListExpr = 0x64600f,
    DesignatedInitExpr = 0x8d017154,
    Designator = 0x6cf40f99, // TODO: vllt. woanders "einsortieren"?
    ArraySubscriptExpr = 0x8c7ab6b2, // TODO: = ArrayAccess?
    ImplicitValueInitExpr = 0xfe7647fa,
    VAArgExpr = 0xdf10fedc,
    BlockExpr = 0xcc75aacd,

    BlockDecl = 0x761e230f,
    FunctionDecl = 0x2a34b689,
    LabelDecl = 0xff6db781,
    EnumDecl = 0xc564aed1,
    EnumConstantDecl = 0x11050d85,
    IndirectFieldDecl = 0x937408ea,
    ValueDecl = 0xbb06d011,
    FileScopeAsmDecl = 0x381879fa,
    CapturedDecl = 0xa3a884ed,

    Attr = 0x56b6cba9,
    InheritableAttr = 0x7c0b04ce,
    InheritableParamAttr = 0x6a4fdb90,

    CompoundStmt = 0x906b6fb4,
    BreakStmt = 0x530ae0a9,
    GCCAsmStmt = 0x652782d6,
    MSAsmStmt = 0xccd123ef,
    AttributedStmt = 0x8e36d148,
    CaptureStmt = 0x1cafe3db,
    IndirectGotoStmt = 0x98888356,

    StructureType = 0xa5b0d36d,
    UnionType = 0x5057c896,

    UnaryOperator = 0x496a1fb5,
    BinaryOperator = 0xa6339d46,
    CompoundAssignmentOperator = 0x9c582bf3,
    AbstractConditionalOperator = 0x151982b7,
    BinaryConditionalOperator = 0x40d2aa93,

    // TODO: sort these:
    ImaginaryLiteral = 0xe340180e,
    OffsetOfNode = 0x17f2d532,
    FieldDecl = 0xac0c83d4,

    RecordDecl = 0x27892cea,
    VectorType = 0x4ed393c3,
    ShuffleVectorExpr = 0x2e2321ad,
    ConvertVectorExpr = 0xfe447195,

    // FIXME
    TypeTraitExpr = 0xe9bda7a,
    ArrayTypeTraitExpr = 0xd6b02f4,
    AsmStmt = 0xe8cca40,
    CXXBoolLiteralExpr = 0x45b2a746,
    CXXCatchStmt = 0xc853e2ac,
    CXXDeleteExpr = 0xbcfa92ec,
    CXXFoldExpr = 0x1cc7935,
    Stmt = 0xc1ac6c2,
    ObjCPropertyRefExpr = 0x6636c2c,
    ObjCIndirectCopyRestoreExpr= 0xb53e833,
    ObjCBridgedCastExpr = 0xcc79223,
    ObjCAtCatchStmt = 0xd6ce349,
    MSDependentExistsStmt = 0xf2097b9,
    LambdaExpr = 0xd799f74,
    GenericSelectionExpr = 0x51b395c,
    ExpressionTraitExpr = 0x8f308a7,
    Expr = 0x821bad1,
// = 0xe8cca403,
// = 0xb190dc73,
// = 0xe9bb85af,
// = 0xa6c4b308,
// = 0xdb0b2b7d,
// = 0x6a185f1b,
// = 0x8c64784e,
// = 0x7d816c99,
// = 0x80161e92,
// = 0xb07dce69,
// = 0x68ce4241,
// = 0x4bfa546,
// = 0x6ab898b3,
// = 0xb5eb2dc1,
// = 0xfbbc5e13,
// = 0xe61ede42,
// = 0x15937adc,
// = 0xf14f6e4c,
// = 0x2598e793,
// = 0x3705a5dc,
// = 0xbf501711,
// = 0x94263251,
// = 0x33510fa8,
// = 0xaef072e3,
// = 0xb4f29497,
// = 0x4daa3158,
// = 0xf612fb23,
// = 0x6d9c820a,

};
}

// FIXME: Some Collectors are broken out of tree.
namespace data_collection {
    static int getMacroStack(SourceLocation, ASTContext&) { return 42; }
}

class TranslationUnitHashVisitor
    : public RecursiveASTVisitor<TranslationUnitHashVisitor> {
    typedef TranslationUnitHashVisitor Inherited;

    ASTContext &Context;

    Hash TopLevelHash;

    /// Counter to handle recursion when hashing function calls
    int ignoreFunctionBody = 0;

    llvm::SmallVector<Hash, 32> HashStack;

    // For the DataCollector, we implement a few addData() functions
    void addData(uint64_t data) { topHash() << data; }
    void addData(const StringRef &str) { topHash() << str;}
    void addData(const QualType&);

public:

#define DEF_ADD_DATA(CLASS, CODE)                                   \
    template<class=void> bool Visit##CLASS(const CLASS *S) {        \
        addData(TranslationUnitHashVisitorPrefix::CLASS);           \
        CODE;                                                       \
        return true;                                                \
    }
#include "StmtDataCollectors.inc"

    TranslationUnitHashVisitor(ASTContext &Context)
        : Context(Context) { }


    // In this storage we save hashes for various memory objects
    std::map<const Type *, Hash::Digest> TypeSilo;
    std::map<const Decl *, Hash::Digest> DeclSilo;

    // For tracking usage of functions/globals
    // Maps each function to all functions called and global variables read
    // by that function
    std::map<const Decl *, std::set<const Decl *>> DefUseSilo;

    // Utilities
    bool hasNodes(const DeclContext *DC);

    void hashDecl(Decl *D);
    void hashStmt(const Stmt *Node);
    void hashType(const QualType &T);
    void hashAttr(const Attr *A);

    void hashName(const NamedDecl *ND);

    void hashCommandLine(const std::list<std::string> &CommandLineArgs);

    std::string getHash(unsigned *ProcessedBytes = nullptr);

    /* For some special nodes, override the traverse function, since we
       need both pre- and post order traversal */
    bool TraverseTranslationUnitDecl(TranslationUnitDecl *Unit);


    bool VisitTypedefType(const TypedefType *T) {
        // For a Typedef Type, we include the typedef declaration
        return TraverseDecl(T->getDecl());
    }

#if 0
    // C Declarations
    bool VisitTranslationUnitDecl(const TranslationUnitDecl *Unit);
    bool VisitVarDecl(const VarDecl *D);
    /// Not interesting
    bool VisitEmptyDecl(const EmptyDecl *) { return true; }
    bool VisitTypedefDecl(const TypedefDecl *) { return true; }

    bool VisitRecordDecl(const RecordDecl *D) { return true; }
    bool VisitFieldDecl(const FieldDecl *D);

    // C Types
  bool VisitBuiltinType(const BuiltinType *T);
  bool VisitPointerType(const PointerType *T);
  bool VisitArrayType(const ArrayType *T);
  bool VisitConstantArrayType(const ConstantArrayType *T);
  bool VisitVariableArrayType(const VariableArrayType *T);
  bool VisitVectorType(const VectorType *);
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
  bool VisitShuffleVectorExpr(const ShuffleVectorExpr *Node);
  bool VisitConvertVectorExpr(const ConvertVectorExpr *Node);

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
#endif


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
  void storeHash(const Type *Obj, Hash::Digest Dig) {
    if (Obj->isPointerType() && inRecordType) {
      // Do not save hashes for pointer types in records, as those values are
      // dummy values
      return;
    }
    TypeSilo[Obj] = Dig;
  }
  void storeHash(const Decl *Obj, Hash::Digest Dig) { DeclSilo[Obj] = Dig; }

  const Hash::Digest *getHash(const Type *Obj) {
    if (Obj->isPointerType() && inRecordType) {
      // Do not load hashes for pointer types inside records, use dummy value
      // instead
      return nullptr;
    }
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

  // Def-Use Silo
  std::stack<const FunctionDecl *>
      CallerFuncs;                        // To keep track of current caller(s)
  std::stack<const VarDecl *> GlobalVars; // To keep track of currently
                                          // initialized global variables

  void storeDefinitionUsage(const Decl *const Used) {
    if (!CallerFuncs.empty())
      DefUseSilo[CallerFuncs.top()].insert(Used);
    else if (!GlobalVars.empty())
      DefUseSilo[GlobalVars.top()].insert(Used);
  }
};

#endif
