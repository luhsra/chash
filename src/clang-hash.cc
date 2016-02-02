#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include "hash-visitor.h"


using namespace clang;

class HashTranslationUnitConsumer : public ASTConsumer {
public:
    virtual void HandleTranslationUnit(clang::ASTContext &Context) {
        // Traversing the translation unit decl via a RecursiveASTVisitor
        // will visit all nodes in the AST.
        Visitor.hashDecl(Context.getTranslationUnitDecl());

        // Context.getTranslationUnitDecl()->dump();

        llvm::errs() << "top-level-hash: " << Visitor.GetHash() << "\n";
    }
  private:
    TranslationUnitHashVisitor Visitor;
};

class HashTranslationUnitAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) {
    return llvm::make_unique<HashTranslationUnitConsumer>();
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) {
    for (unsigned i = 0, e = args.size(); i != e; ++i) {
      llvm::errs() << " arg = " << args[i] << "\n";

      // Example error handling.
      if (args[i] == "-an-error") {
        DiagnosticsEngine &D = CI.getDiagnostics();
        unsigned DiagID = D.getCustomDiagID(DiagnosticsEngine::Error,
                                            "invalid argument '%0'");
        D.Report(DiagID) << args[i];
        return false;
      }
    }
    if (args.size() && args[0] == "help") {
        // FIXME
        PrintHelp(llvm::errs());
    }

    return true;
  }
  void PrintHelp(llvm::raw_ostream &ros) {
    ros << "Help for PrintFunctionNames plugin goes here\n";
  }
};

static FrontendPluginRegistry::Add<HashTranslationUnitAction>
    X("hash-unit", "hash translation uni");
