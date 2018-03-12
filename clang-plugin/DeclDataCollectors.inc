DEF_ADD_DATA(BlockDecl, {

      // CrossRef
      auto it = llvm::make_range(S->capture_begin(), S->capture_end());
      addData(it);
   }
)
DEF_ADD_DATA(CXXConstructorDecl, {

       // CrossRef
       addData(S->inits());
  }
)
DEF_ADD_DATA(CXXRecordDecl, {

       // CrossRef
       if (S->isCompleteDefinition()) {
           addData(S->bases());
       }
  }
)
DEF_ADD_DATA(CapturedDecl, {

      addData(S->isNothrow());
  }
)
DEF_ADD_DATA(Decl, {

      // Every Declaration gets a tag field in the hash stream. It is
      // hashed to add additional randomness to the hash
      addData(llvm::hash_value(S->getKind()));

      // CrossRef
      addData(S->hasAttrs());
      if (S->hasAttrs())
        addData(S->attrs());
  }
)
DEF_ADD_DATA(DeclContext, {

      // CrossRef
      addData(S->decls());
   }
)
DEF_ADD_DATA(DecompositionDecl, {

      // CrossRef
      addData(S->bindings());
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
DEF_ADD_DATA(FriendTemplateDecl, {

      // CrossRef
      addData(S->getNumTemplateParameters());
      for (unsigned I = 0, E = S->getNumTemplateParameters(); I < E; ++I) {
          auto TPL = S->getTemplateParameterList(I);
          auto it  = llvm::make_range(TPL->begin(), TPL->end());
          addData(it);
      }
  }
)
DEF_ADD_DATA(FunctionDecl, {

       addData(S->isExternC());
       addData(S->isGlobal());
       addData(S->isNoReturn());
       addData(S->getStorageClass());
       addData(S->isInlineSpecified());
       addData(S->isInlined());

       // CrossRef
       auto range = llvm::make_range(S->param_begin(), S->param_end());
       addData(range);
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

      addData(S->getNameAsString());
  }
)
DEF_ADD_DATA(ObjCCategoryDecl, {

     // CrossRef
     if (auto *it = S->getTypeParamList()) {
       auto range = llvm::make_range(it->begin(), it->end());
       addData(range);
     }
  }
)
DEF_ADD_DATA(ObjCInterfaceDecl, {

     // CrossRef
     if (auto *it = S->getTypeParamListAsWritten()) {
       auto range = llvm::make_range(it->begin(), it->end());
       addData(range);
     }
  }
)
DEF_ADD_DATA(ObjCMethodDecl, {

     // CrossRef
     auto range = llvm::make_range(S->param_begin(), S->param_end());
     addData(range);
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
