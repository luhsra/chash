#ifndef __HASH_VISITOR
#define __HASH_VISITOR

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <string>

class TranslationUnitHashVisitor
    : public clang::RecursiveASTVisitor<TranslationUnitHashVisitor> {
public:
    bool VisitVarDecl(clang::VarDecl *);

    std::string GetHash();
};

#endif
