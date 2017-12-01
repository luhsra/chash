class Stmt {
  code Code = [{
    addData(S->getStmtClass());
    // This ensures that non-macro-generated code isn't identical to
    // macro-generated code.
    addData(data_collection::getMacroStack(S->getLocStart(), Context));
    addData(data_collection::getMacroStack(S->getLocEnd(), Context));
  }];
}

class Expr {
  code Code = [{
    addData(S->getType());
  }];
}

//--- Builtin functionality ----------------------------------------------//
class ArrayTypeTraitExpr {
  code Code = [{
    addData(S->getTrait());
  }];
}
class ExpressionTraitExpr {
  code Code = [{
    addData(S->getTrait());
  }];
}
class PredefinedExpr {
  code Code = [{
    addData(S->getIdentType());
    addData(S->getFunctionName()->getString());
  }];
}
class TypeTraitExpr {
  code Code = [{
    addData(S->getTrait());
    for (unsigned i = 0; i < S->getNumArgs(); ++i)
      addData(S->getArg(i)->getType());
  }];
}

class UnaryExprOrTypeTraitExpr {
  code Code = [{
    addData(S->getKind());
    if (S->isArgumentType()) {
       addData(S->getArgumentType());
    }
  }];
}


//--- Calls --------------------------------------------------------------//
class CallExpr {
  code Code = [{
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
  }];
}

//--- Value references ---------------------------------------------------//
class DeclRefExpr {
  code Code = [{
    addData(S->getDecl()->getQualifiedNameAsString());
  }];
}
class MemberExpr {
  code Code = [{
    addData(S->isArrow());
    addData(S->getMemberDecl()->getName());
  }];
}

//--- Literals -----------------------------------------------------------//
class IntegerLiteral {
  code Code = [{
    addData(llvm::hash_value(S->getValue()));
  }];
}
class FloatingLiteral {
  code Code = [{
    addData(llvm::hash_value(S->getValue()));
  }];
}
class StringLiteral {
  code Code = [{
    addData(S->getString());
}];
}
class CXXBoolLiteralExpr {
  code Code = [{
    addData(S->getValue());
  }];
}
class CharacterLiteral {
  code Code = [{
    addData(S->getValue());
  }];
}

//--- Exceptions ---------------------------------------------------------//
class CXXCatchStmt {
  code Code = [{
    addData(S->getCaughtType());
  }];
}

//--- C++ OOP Stmts ------------------------------------------------------//
class CXXDeleteExpr {
  code Code = [{
    addData(S->isArrayFormAsWritten()); addData(S->isGlobalDelete());
  }];
}

//--- Casts --------------------------------------------------------------//
class ObjCBridgedCastExpr {
  code Code = [{
    addData(S->getBridgeKind());
  }];
}

class CastExpr {
  code Code = [{
    addData(S->getCastKind());
  }];
}

//--- Miscellaneous Exprs ------------------------------------------------//
class BinaryOperator {
  code Code = [{
    addData(S->getOpcode());
  }];
}
class UnaryOperator {
  code Code = [{
    addData(S->getOpcode());
  }];
}

class VAArgExpr {
  code Code = [{
       addData(S->isMicrosoftABI());
  }];
}

//--- Control flow -------------------------------------------------------//
class GotoStmt {
  code Code = [{
    addData(S->getLabel()->getName());
  }];
}
class IndirectGotoStmt {
  code Code = [{
    if (S->getConstantTarget())
      addData(S->getConstantTarget()->getName());
  }];
}
class LabelStmt {
  code Code = [{
    addData(S->getDecl()->getName());
  }];
}
class MSDependentExistsStmt {
  code Code = [{
    addData(S->isIfExists());
  }];
}
class AddrLabelExpr {
  code Code = [{
    addData(S->getLabel()->getName());
  }];
}

//--- Objective-C --------------------------------------------------------//
class ObjCIndirectCopyRestoreExpr {
  code Code = [{
    addData(S->shouldCopy());
  }];
}
class ObjCPropertyRefExpr {
  code Code = [{
    addData(S->isSuperReceiver()); addData(S->isImplicitProperty());
  }];
}
class ObjCAtCatchStmt {
  code Code = [{
    addData(S->hasEllipsis());
  }];
}

//--- Miscellaneous Stmts ------------------------------------------------//
class CXXFoldExpr {
  code Code = [{
    addData(S->isRightFold()); addData(S->getOperator());
  }];
}
class GenericSelectionExpr {
  code Code = [{
    for (unsigned i = 0; i < S->getNumAssocs(); ++i) {
      addData(S->getAssocType(i));
    }
  }];
}
class LambdaExpr {
  code Code = [{
    for (const LambdaCapture &C : S->captures()) {
      addData(C.isPackExpansion());
      addData(C.getCaptureKind());
      if (C.capturesVariable())
        addData(C.getCapturedVar()->getType());
    }
    addData(S->isGenericLambda());
    addData(S->isMutable());
  }];
}
class DeclStmt {
  code Code = [{
    auto numDecls = std::distance(S->decl_begin(), S->decl_end());
    addData(static_cast<unsigned>(numDecls));
    // FIXME?
    for (const Decl *D : S->decls()) {
      if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
        addData(VD->getType());
      }
    }
  }];
}
class AsmStmt {
  code Code = [{
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
  }];
}
class AttributedStmt {
  code Code = [{
    for (const Attr *A : S->getAttrs()) {
       // We duplicate class Attr here to not rely on being integrated
       // into a RecursiveASTVisitor.
       std::string AttrString;      
       llvm::raw_string_ostream OS(AttrString);
       A->printPretty(OS, Context.getLangOpts());
       OS.flush();
      addData(std::string(A->getSpelling()));
    }
  }];
}

class CompoundStmt {
  code Code = [{
    addData(S->size());
  }];
}


//--- Attributes ---------------------------------------------------------//
class Attr {
  code Code = [{
    std::string AttrString;      
    llvm::raw_string_ostream OS(AttrString);
    S->printPretty(OS, Context.getLangOpts());
    OS.flush();
    addData(AttrString);
  }];
}
//--- Declarations -------------------------------------------------------//

class ValueDecl  {
  code Code = [{
      /* Field Declarations can induce recursions */
      if (isa<FieldDecl>(S)) {
        addData(std::string(S->getType().getAsString()));
      } else {
        addData(S->getType());
      }
      addData(S->isWeak());
  }];
}

class NamedDecl {
  code Code = [{
      addData(S->getName());
  }];
}

class TypeDecl {
  code Code = [{
      // If we would hash the resulting type for a typedef, we would get into an endless recursion.
      if (!isa<TypedefNameDecl>(S)
          && !isa<RecordDecl>(S)
          && !isa<EnumDecl>(S)) {
         addData(QualType(S->getTypeForDecl(),0));
      }
  }];
}

class EnumDecl {
  code Code = [{
      addData(S->getNumPositiveBits());
      addData(S->getNumNegativeBits());
  }];
}

class EnumConstantDecl {
  code Code = [{
       /* Not every enum has a init expression. Therefore, 
          we extract the actual enum value from it. */
       addData(S->getInitVal().getExtValue());
  }];
}

class TagDecl {
  code Code = [{
     addData(S->getTagKind());
  }];
}


class TypedefNameDecl {
  code Code = [{
     addData(S->getUnderlyingType());
  }];
}

class VarDecl {
  code Code = [{
      addData(S->getStorageClass());
      addData(S->getTLSKind());
      addData(S->isModulePrivate());
      addData(S->isNRVOVariable());
  }];
}

class ParmVarDecl {
  code Code = [{
       addData(S->isParameterPack());
       addData(S->getOriginalType());
  }];
}

class ImplicitParamDecl {
  code Code = [{
       addData(S->getParameterKind());
  }];
}

class FunctionDecl {
  code Code = [{
       addData(S->isExternC());
       addData(S->isGlobal());
       addData(S->isNoReturn());
       addData(S->getStorageClass());
       addData(S->isInlineSpecified());
       addData(S->isInlined());
  }];
}

class LabelDecl {
  code Code = [{
       addData(S->isGnuLocal());
       addData(S->isMSAsmLabel());
       if (S->isMSAsmLabel()) {
          addData(S->getMSAsmLabel());
       }
  }];
}

class FieldDecl {
  code Code = [{
      addData(S->isBitField());
  }];
}

class CapturedDecl {
  code Code = [{
      addData(S->isNothrow());
  }];
}


//--- Types ---------------------------------------------------------------//

class Type {
  code Code = [{
     addData(S->getTypeClass());
  }];
}

class BuiltinType {
   code Code = [{
      addData(S->getKind());
   }];
}

class ArrayType  {
   code Code = [{
      addData(S->getSizeModifier());
      addData(S->getIndexTypeCVRQualifiers());
   }];
}

class ConstantArrayType {
   code Code = [{
      addData(S->getSize().getZExtValue());
   }];
}

class VectorType {
   code Code = [{
      addData(S->getNumElements());
      addData(S->getVectorKind());
   }];
}

class FunctionType {
   code Code = [{
      addData(S->getRegParmType());
      addData(S->getCallConv());
   }];
}

class FunctionProtoType {
   code Code = [{
      addData(S->getExceptionSpecType());
      addData(S->isVariadic());
      addData(S->getRefQualifier());
      addData(S->hasTrailingReturn());
   }];
}

class UnaryTransformType {
   code Code = [{
        addData(S->getUTTKind());
   }];
}

class AttributedType {
   code Code = [{
        addData(S->getAttrKind());
   }];
}

class ElaboratedType {
   code Code = [{
        addData(S->getKeyword());
   }];
}






