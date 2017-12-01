DEF_ADD_DATA(CapturedDecl, {

      addData(S->isNothrow());
  }
)
DEF_ADD_DATA(EnumConstantDecl, {

       /* Not every enum has a init expression. Therefore, 
          we extract the actual enum value from it. */
       addData(S->getInitVal().getExtValue());
  }
)
DEF_ADD_DATA(EnumDecl, {

      addData(S->getNumPositiveBits());
      addData(S->getNumNegativeBits());
  }
)
DEF_ADD_DATA(FieldDecl, {

      addData(S->isBitField());
  }
)
DEF_ADD_DATA(FunctionDecl, {

       addData(S->isExternC());
       addData(S->isGlobal());
       addData(S->isNoReturn());
       addData(S->getStorageClass());
       addData(S->isInlineSpecified());
       addData(S->isInlined());
  }
)
DEF_ADD_DATA(ImplicitParamDecl, {

       addData(S->getParameterKind());
  }
)
DEF_ADD_DATA(LabelDecl, {

       addData(S->isGnuLocal());
       addData(S->isMSAsmLabel());
       if (S->isMSAsmLabel()) {
          addData(S->getMSAsmLabel());
       }
  }
)
DEF_ADD_DATA(NamedDecl, {

      addData(S->getName());
  }
)
DEF_ADD_DATA(ParmVarDecl, {

       addData(S->isParameterPack());
       addData(S->getOriginalType());
  }
)
DEF_ADD_DATA(TagDecl, {

     addData(S->getTagKind());
  }
)
DEF_ADD_DATA(TypeDecl, {

      addData(QualType(S->getTypeForDecl(),0));
  }
)
DEF_ADD_DATA(TypedefNameDecl, {

     addData(S->getUnderlyingType());
  }
)
DEF_ADD_DATA(ValueDecl, {

      addData(S->getType());
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
#undef DEF_ADD_DATA
