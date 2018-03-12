DEF_ADD_DATA(ArrayType, {

      addData(S->getSizeModifier());
      addData(S->getIndexTypeCVRQualifiers());
   }
)
DEF_ADD_DATA(AttributedType, {

        addData(S->getAttrKind());
   }
)
DEF_ADD_DATA(BuiltinType, {

      addData(S->getKind());
   }
)
DEF_ADD_DATA(ConstantArrayType, {

      addData(S->getSize().getZExtValue());
   }
)
DEF_ADD_DATA(ElaboratedType, {

        addData(S->getKeyword());
   }
)
DEF_ADD_DATA(FunctionProtoType, {

      addData(S->getExceptionSpecType());
      addData(S->isVariadic());
      addData(S->getRefQualifier());
      addData(S->hasTrailingReturn());

      addData(S->param_types());
      addData(S->exceptions());
   }
)
DEF_ADD_DATA(FunctionType, {

      addData(S->getRegParmType());
      addData(S->getCallConv());
   }
)
DEF_ADD_DATA(ObjCObjectType, {

        addData(S->getTypeArgsAsWritten());
   }
)
DEF_ADD_DATA(Type, {

     addData(llvm::hash_value(S->getTypeClass()));
  }
)
DEF_ADD_DATA(UnaryTransformType, {

        addData(S->getUTTKind());
   }
)
DEF_ADD_DATA(VectorType, {

      addData(S->getNumElements());
      addData(S->getVectorKind());
   }
)
#undef DEF_ADD_DATA
