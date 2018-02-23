DEF_ADD_DATA(AddrLabelExpr, {

    addData(S->getLabel()->getName());
  }
)
DEF_ADD_DATA(ArrayTypeTraitExpr, {

    addData(S->getTrait());
  }
)
DEF_ADD_DATA(AsmStmt, {

    addData(S->isSimple());
    addData(S->isVolatile());
    addData(S->generateAsmString(Context));

    // CrossRef + FIXME
    addData(S->getNumInputs());
    addData(S->getNumOutputs());
    addData(S->getNumClobbers());

    for (unsigned i = 0; i < S->getNumInputs(); ++i) {
      addData(S->getInputConstraint(i));
    }
    for (unsigned i = 0; i < S->getNumOutputs(); ++i) {
      addData(S->getOutputConstraint(i));
    }
    for (unsigned i = 0; i < S->getNumClobbers(); ++i) {
      addData(S->getClobber(i));
    }
  }
)
DEF_ADD_DATA(AttributedStmt, {

    for (const Attr *A : S->getAttrs()) {
       // We duplicate class Attr here to not rely on being integrated
       // into a RecursiveASTVisitor.
       std::string AttrString;      
       llvm::raw_string_ostream OS(AttrString);
       A->printPretty(OS, Context.getLangOpts());
       OS.flush();
      addData(std::string(A->getSpelling()));
    }
  }
)
DEF_ADD_DATA(BinaryOperator, {

    addData(S->getOpcode());
  }
)
DEF_ADD_DATA(CXXBoolLiteralExpr, {

    addData(S->getValue());
  }
)
DEF_ADD_DATA(CXXCatchStmt, {

    addData(S->getCaughtType());
  }
)
DEF_ADD_DATA(CXXDeleteExpr, {

    addData(S->isArrayFormAsWritten()); addData(S->isGlobalDelete());
  }
)
DEF_ADD_DATA(CXXFoldExpr, {

    addData(S->isRightFold()); addData(S->getOperator());
  }
)
DEF_ADD_DATA(CallExpr, {

    // Function pointers don't have a callee and we just skip hashing it.
    if (const FunctionDecl *D = S->getDirectCallee()) {
      // If the function is a template specialization, we also need to handle
      // the template arguments as they are not included in the qualified name.
      if (auto Args = D->getTemplateSpecializationArgs()) {
        std::string ArgString;

        // Print all template arguments into ArgString
        llvm::raw_string_ostream OS(ArgString);
        for (unsigned i = 0; i < Args->size(); ++i) {
          Args->get(i).print(Context.getLangOpts(), OS);
          // Add a padding character so that 'foo<X, XX>()' != 'foo<XX, X>()'.
          OS << '\n';
        }
        OS.flush();

        addData(ArgString);
      }
      addData(D->getQualifiedNameAsString());
    }
  }
)
DEF_ADD_DATA(CastExpr, {

    addData(S->getCastKind());
  }
)
DEF_ADD_DATA(CharacterLiteral, {

    addData(S->getValue());
  }
)
DEF_ADD_DATA(CompoundStmt, {

    // CrossRef
    addData(S->size());
  }
)
DEF_ADD_DATA(DeclRefExpr, {

    addData(S->getDecl()->getQualifiedNameAsString());
  }
)
DEF_ADD_DATA(DeclStmt, {

    // CrossRef
    addData(S->decls());

    // FIXME? As this should be done by a using visitor
    for (const Decl *D : S->decls()) {
      if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
        addData(VD->getType());
      }
    }
  }
)
DEF_ADD_DATA(Expr, {

    addData(S->getType());
  }
)
DEF_ADD_DATA(ExpressionTraitExpr, {

    addData(S->getTrait());
  }
)
DEF_ADD_DATA(FloatingLiteral, {

    addData(llvm::hash_value(S->getValue()));
  }
)
DEF_ADD_DATA(GenericSelectionExpr, {

    // CrossRef
    addData(S->getNumAssocs());
    for (unsigned i = 0; i < S->getNumAssocs(); ++i) {
      addData(S->getAssocType(i));
    }
  }
)
DEF_ADD_DATA(GotoStmt, {

    addData(S->getLabel()->getName());
  }
)
DEF_ADD_DATA(IndirectGotoStmt, {

    if (S->getConstantTarget())
      addData(S->getConstantTarget()->getName());
  }
)
DEF_ADD_DATA(IntegerLiteral, {

    addData(llvm::hash_value(S->getValue()));
  }
)
DEF_ADD_DATA(LabelStmt, {

    addData(S->getDecl()->getName());
  }
)
DEF_ADD_DATA(LambdaExpr, {

    addData(S->isGenericLambda());
    addData(S->isMutable());
    addData(S->hasExplicitParameters());
    addData(S->hasExplicitResultType());


    // CrossRef
    addData(S->captures());
    addData(S->explicit_captures());
    for (const LambdaCapture &C : S->captures()) {
      addData(C.isPackExpansion());
      addData(C.getCaptureKind());
      if (C.capturesVariable())
        addData(C.getCapturedVar()->getType());
    }
    

  }
)
DEF_ADD_DATA(MSDependentExistsStmt, {

    addData(S->isIfExists());
  }
)
DEF_ADD_DATA(MemberExpr, {

    // I suspect this should be included: addData(S->isArrow());
    addData(S->getMemberDecl()->getName());
  }
)
DEF_ADD_DATA(ObjCAtCatchStmt, {

    addData(S->hasEllipsis());
  }
)
DEF_ADD_DATA(ObjCBridgedCastExpr, {

    addData(S->getBridgeKind());
  }
)
DEF_ADD_DATA(ObjCIndirectCopyRestoreExpr, {

    addData(S->shouldCopy());
  }
)
DEF_ADD_DATA(ObjCPropertyRefExpr, {

    addData(S->isSuperReceiver()); addData(S->isImplicitProperty());
  }
)
DEF_ADD_DATA(PredefinedExpr, {

    addData(S->getIdentType());
    addData(S->getFunctionName()->getString());
  }
)
DEF_ADD_DATA(PseudoObjectExpr, {

    // CrossRef
    addData(S->semantics());
  }
)
DEF_ADD_DATA(Stmt, {

    addData(S->getStmtClass());
    // This ensures that non-macro-generated code isn't identical to
    // macro-generated code.
    addData(data_collection::getMacroStack(S->getLocStart(), Context));
    addData(data_collection::getMacroStack(S->getLocEnd(), Context));

    // CrossRef
    addData(S->children());
  }
)
DEF_ADD_DATA(StringLiteral, {

    addData(S->getString());
}
)
DEF_ADD_DATA(TypeTraitExpr, {

    addData(S->getTrait());
    // CrossRef
    addData(S->getNumArgs());
    for (unsigned i = 0; i < S->getNumArgs(); ++i)
      addData(S->getArg(i)->getType());
  }
)
DEF_ADD_DATA(UnaryExprOrTypeTraitExpr, {

    addData(S->getKind());
    if (S->isArgumentType()) {
       addData(S->getArgumentType());
    }
  }
)
DEF_ADD_DATA(UnaryOperator, {

    addData(S->getOpcode());
  }
)
DEF_ADD_DATA(VAArgExpr, {

       addData(S->isMicrosoftABI());
  }
)
#undef DEF_ADD_DATA
