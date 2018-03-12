//--- Declarations -------------------------------------------------------//

class Decl {
  code Code = [{
      // Every Declaration gets a tag field in the hash stream. It is
      // hashed to add additional randomness to the hash
      addData(llvm::hash_value(S->getKind()));

      // CrossRef
      addData(S->hasAttrs());
      if (S->hasAttrs())
        addData(S->attrs());
  }];
}

class DeclContext  {
   code Code = [{
      // CrossRef
      addData(S->decls());
   }];
}

class BlockDecl  {
   code Code = [{
      // CrossRef
      auto it = llvm::make_range(S->capture_begin(), S->capture_end());
      addData(it);
   }];
}


class ValueDecl  {
  code Code = [{
      addData(S->getType());
      addData(S->isWeak());
  }];
}

class NamedDecl {
  code Code = [{
      addData(S->getNameAsString());
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

       // CrossRef
       auto range = llvm::make_range(S->param_begin(), S->param_end());
       addData(range);
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

class CXXRecordDecl {
  code Code = [{
       // CrossRef
       if (S->isCompleteDefinition()) {
           addData(S->bases());
       }
  }];
}

class CXXConstructorDecl {
  code Code = [{
       // CrossRef
       addData(S->inits());
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

class DecompositionDecl {
  code Code = [{
      // CrossRef
      addData(S->bindings());
  }];
}


//--- Obj-C ---------------------------------------------------------//

class ObjCCategoryDecl {
  code Code = [{
     // CrossRef
     if (auto *it = S->getTypeParamList()) {
       auto range = llvm::make_range(it->begin(), it->end());
       addData(range);
     }
  }];
}

class ObjCInterfaceDecl {
  code Code = [{
     // CrossRef
     if (auto *it = S->getTypeParamListAsWritten()) {
       auto range = llvm::make_range(it->begin(), it->end());
       addData(range);
     }
  }];
}

class ObjCMethodDecl {
  code Code = [{
     // CrossRef
     auto range = llvm::make_range(S->param_begin(), S->param_end());
     addData(range);
  }];
}

//--- Templates -----------------------------------------------------//

class FriendTemplateDecl {
  code Code = [{
      // CrossRef
      addData(S->getNumTemplateParameters());
      for (unsigned I = 0, E = S->getNumTemplateParameters(); I < E; ++I) {
          auto TPL = S->getTemplateParameterList(I);
          auto it  = llvm::make_range(TPL->begin(), TPL->end());
          addData(it);
      }
  }];
}

