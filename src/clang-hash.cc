#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include "hash-visitor.h"


using namespace clang;
using namespace llvm;

class HashTranslationUnitConsumer : public ASTConsumer {
public:
    HashTranslationUnitConsumer(raw_ostream *OS) : toplevel_hash_stream(OS) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context) {
        // Traversing the translation unit decl via a RecursiveASTVisitor
        // will visit all nodes in the AST.
        Visitor.hashDecl(Context.getTranslationUnitDecl());

        // Context.getTranslationUnitDecl()->dump();
        if (toplevel_hash_stream) {
            std::string hash = Visitor.GetHash();
            toplevel_hash_stream->write(hash.c_str(), hash.length());

            delete toplevel_hash_stream;
        }
        llvm::errs() << "top-level-hash: " << Visitor.GetHash() << "\n";
    }
  private:
    raw_ostream *toplevel_hash_stream;
    TranslationUnitHashVisitor Visitor;
};

class HashTranslationUnitAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) {

      // Write hash database to .o.hash if the compiler produces a object file
      llvm::raw_ostream *Out = nullptr;
      if (CI.getFrontendOpts().OutputFile != "") {
          std::error_code Error;
          std::string HashFile = CI.getFrontendOpts().OutputFile + ".hash";
          Out = new llvm::raw_fd_ostream(HashFile, Error, llvm::sys::fs::F_Text);
          errs() << "dump-ast-file: " << CI.getFrontendOpts().OutputFile << " " << HashFile << "\n";
          if (Error) {
              errs() << "Could not open ast-hash file: " << CI.getFrontendOpts().OutputFile << "\n";
          }
      }
      return llvm::make_unique<HashTranslationUnitConsumer>(Out);
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
    X("hash-unit", "hash translation unit");
