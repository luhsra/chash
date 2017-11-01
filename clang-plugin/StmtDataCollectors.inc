DEF_ADD_DATA(AddrLabelExpr, {

    addData(S->getLabel()->getName());
  }
)
DEF_ADD_DATA(AdjustedType, {
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
DEF_ADD_DATA(AtomicType, {
 }
)
DEF_ADD_DATA(Attr, {

    std::string AttrString;      
    // Print all template arguments into ArgString
    llvm::raw_string_ostream OS(AttrString);
    S->printPretty(OS, Context.getLangOpts());
    OS.flush();
    std::cerr << " ATTR:" << AttrString << "\n";
    addData(AttrString);
  }
)
DEF_ADD_DATA(AttributedStmt, {

    for (const Attr *A : S->getAttrs()) {
      addData(std::string(A->getSpelling()));
    }
  }
)
DEF_ADD_DATA(AttributedType, {

        addData(S->getAttrKind());
   }
)
DEF_ADD_DATA(AutoType, {
 }
)
DEF_ADD_DATA(BinaryOperator, {

    addData(S->getOpcode());
  }
)
DEF_ADD_DATA(BlockPointerType, {
 }
)
DEF_ADD_DATA(BuiltinType, {

      addData(S->getKind());
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
DEF_ADD_DATA(CharacterLiteral, {

    addData(S->getValue());
  }
)
DEF_ADD_DATA(ComplexType, {
 }
)
DEF_ADD_DATA(ConstantArrayType, {

      addData(S->getSize().getZExtValue());
   }
)
DEF_ADD_DATA(DecayedType, {
 }
)
DEF_ADD_DATA(DeclRefExpr, {

    addData(S->getDecl()->getQualifiedNameAsString());
  }
)
DEF_ADD_DATA(DeclStmt, {

    auto numDecls = std::distance(S->decl_begin(), S->decl_end());
    addData(static_cast<unsigned>(numDecls));
    // FIXME?
    for (const Decl *D : S->decls()) {
      if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
        addData(VD->getType());
      }
    }
  }
)
DEF_ADD_DATA(DecltypeType, {
 }
)
DEF_ADD_DATA(DependentSizedArrayType, {
 }
)
DEF_ADD_DATA(DependentSizedExtVectorType, {
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
DEF_ADD_DATA(ExtVectorType, {
 }
)
DEF_ADD_DATA(FloatingLiteral, {

    addData(llvm::hash_value(S->getValue()));
  }
)
DEF_ADD_DATA(FunctionNoProtoType, {
 }
)
DEF_ADD_DATA(FunctionProtoType, {

      addData(S->getExceptionSpecType());
      addData(S->isVariadic());
      addData(S->getRefQualifier());
      addData(S->hasTrailingReturn());
   }
)
DEF_ADD_DATA(FunctionType, {

      addData(S->getRegParmType());
      addData(S->getCallConv());
   }
)
DEF_ADD_DATA(GenericSelectionExpr, {

    for (unsigned i = 0; i < S->getNumAssocs(); ++i) {
      addData(S->getAssocType(i));
    }
  }
)
DEF_ADD_DATA(GotoStmt, {

    addData(S->getLabel()->getName());
  }
)
DEF_ADD_DATA(IncompleteArrayType, {
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
DEF_ADD_DATA(LValueReferenceType, {
 }
)
DEF_ADD_DATA(LabelStmt, {

    addData(S->getDecl()->getName());
  }
)
DEF_ADD_DATA(LambdaExpr, {

    for (const LambdaCapture &C : S->captures()) {
      addData(C.isPackExpansion());
      addData(C.getCaptureKind());
      if (C.capturesVariable())
        addData(C.getCapturedVar()->getType());
    }
    addData(S->isGenericLambda());
    addData(S->isMutable());
  }
)
DEF_ADD_DATA(MSDependentExistsStmt, {

    addData(S->isIfExists());
  }
)
DEF_ADD_DATA(MemberExpr, {

    addData(S->getMemberDecl()->getName());
  }
)
DEF_ADD_DATA(MemberPointerType, {
 }
)
DEF_ADD_DATA(NamedDecl, {

      addData(S->getName());
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
DEF_ADD_DATA(PipeType, {
 }
)
DEF_ADD_DATA(PointerType, {
 }
)
DEF_ADD_DATA(PredefinedExpr, {

    addData(S->getIdentType());
  }
)
DEF_ADD_DATA(RValueReferenceType, {
 }
)
DEF_ADD_DATA(Stmt, {

    addData(S->getStmtClass());
    // This ensures that non-macro-generated code isn't identical to
    // macro-generated code.
    addData(data_collection::getMacroStack(S->getLocStart(), Context));
    addData(data_collection::getMacroStack(S->getLocEnd(), Context));
  }
)
DEF_ADD_DATA(StringLiteral, {

    addData(S->getString());
}
)
DEF_ADD_DATA(TagDecl, {

     addData(S->getTagKind());
  }
)
DEF_ADD_DATA(TypeDecl, {

      // If we would hash the resulting type for a typedef, we would get into an endless recursion.
      if (!isa<TypedefNameDecl>(S)
          && !isa<RecordDecl>(S)
          && !isa<EnumDecl>(S)) {
         addData(QualType(S->getTypeForDecl(),0));
      }
  }
)
DEF_ADD_DATA(TypeOfExprType, {
 }
)
DEF_ADD_DATA(TypeOfType, {
 }
)
DEF_ADD_DATA(TypeTraitExpr, {

    addData(S->getTrait());
    for (unsigned i = 0; i < S->getNumArgs(); ++i)
      addData(S->getArg(i)->getType());
  }
)
DEF_ADD_DATA(TypedefNameDecl, {

     addData(S->getUnderlyingType());
  }
)
DEF_ADD_DATA(UnaryOperator, {

    addData(S->getOpcode());
  }
)
DEF_ADD_DATA(UnaryTransformType, {

        addData(S->getUTTKind());
   }
)
DEF_ADD_DATA(ValueDecl, {

      /* Field Declarations can induce recursions */
      if (isa<FieldDecl>(S)) {
        addData(std::string(S->getType().getAsString()));
      } else {
        addData(S->getType());
      }
      addData(S->isWeak());
  }
)
DEF_ADD_DATA(VarDecl, {

      addData(S->getStorageClass());
      addData(S->getTLSKind());
      addData(S->isModulePrivate());
      addData(S->isNRVOVariable());
  }
)
DEF_ADD_DATA(VariableArrayType, {
 }
)
DEF_ADD_DATA(VectorType, {

      addData(S->getNumElements());
      addData(S->getVectorKind());
   }
)
#undef DEF_ADD_DATA
