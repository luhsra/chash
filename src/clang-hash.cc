#include <sstream>
#include <string>
#include <iostream>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory ClangASTHashCategory("Clang AST Hash");

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
    MyASTVisitor()  {}

    bool VisitStmt(Stmt *s) {
        // Only care about If statements.
        if (isa<IfStmt>(s)) {
            IfStmt *IfStatement = cast<IfStmt>(s);
        }

        return true;
    }

    bool VisitRecordDecl(RecordDecl *rd) {
        rd->dump();
        return true;
    }

    bool VisitFieldDecl(FieldDecl *fd) {
        // Only function definitions (with bodies), not declarations.
        if (fd->isAnonymousStructOrUnion()) {
            std::cout << "Anonymous Struct" << std::endl;
        }
        const clang::QualType qt = fd->getType().getCanonicalType();
        const clang::Type* t = qt.getTypePtr();
        while (isa<clang::PointerType>(t)) {
            t = t->getPointeeType().getTypePtr();
        }
        fd->dump();
        t->dump();
        return true;
    }
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer() {}

    // Override the method that gets called for each parsed top-level
    // declaration.
    bool HandleTopLevelDecl(DeclGroupRef DR) override {
        for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b) {
            // Traverse the declaration using our AST visitor.
            Visitor.TraverseDecl(*b);
        }
        return true;
    }

private:
    MyASTVisitor Visitor;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
    MyFrontendAction() {}
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   StringRef file) override {
        llvm::errs() << "** Creating AST consumer for: " << file << "\n";
        return llvm::make_unique<MyASTConsumer>();
    }

private:
};

int main(int argc, const char **argv) {
    CommonOptionsParser op(argc, argv, ClangASTHashCategory);
    ClangTool Tool(op.getCompilations(), op.getSourcePathList());

    // ClangTool::run accepts a FrontendActionFactory, which is then used to
    // create new objects implementing the FrontendAction interface. Here we use
    // the helper newFrontendActionFactory to create a default factory that will
    // return a new MyFrontendAction object every time.
    // To further customize this, we could create our own factory class.
    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
