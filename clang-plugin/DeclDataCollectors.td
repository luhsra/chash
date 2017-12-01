//--- Declarations -------------------------------------------------------//

class ValueDecl  {
  code Code = [{
      addData(S->getType());
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
      addData(QualType(S->getTypeForDecl(),0));
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


