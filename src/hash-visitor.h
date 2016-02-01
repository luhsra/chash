#ifndef __HASH_VISITOR
#define __HASH_VISITOR

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <string>

#include "SHA1.h"

class TranslationUnitHashVisitor
    : public clang::RecursiveASTVisitor<TranslationUnitHashVisitor> {
public:
    bool VisitVarDecl(clang::VarDecl *);

    std::string GetHash();

private:
    sha1::SHA1 toplevel_hash;
};

#endif
