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
