#include "hash-visitor.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <chrono>
#include <fstream>
#include <unistd.h>
#include <utime.h>
#include <sys/stat.h>
#include <sys/types.h>


using namespace clang;
using namespace llvm;

static std::chrono::high_resolution_clock::time_point StartCompilation;

static char *objectfile = NULL;
static char *objectfile_copy = NULL;
static void link_object_file() {
    if (objectfile && objectfile_copy) {
        link(objectfile, objectfile_copy);
    }
}

class HashTranslationUnitConsumer : public ASTConsumer {
public:
   HashTranslationUnitConsumer(raw_ostream *HS, raw_ostream *OS,
                               const std::string &PrevHashString,
                               const std::string &OutFile,
                               bool StopIfSameHash)
      : TopLevelHashStream(HS), Terminal(OS),
        PreviousHashString(PrevHashString),
        OutputFile(OutFile), StopIfSameHash(StopIfSameHash) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) override {
    const auto StartHashing = std::chrono::high_resolution_clock::now();

    // Traversing the translation unit decl via a RecursiveASTVisitor
    // will visit all nodes in the AST.
    Visitor.hashDecl(Context.getTranslationUnitDecl());

    hashCommandLineArguments();

    const auto FinishHashing = std::chrono::high_resolution_clock::now();

    // Context.getTranslationUnitDecl()->dump();
    unsigned ProcessedBytes;
    const std::string HashString = Visitor.getHash(&ProcessedBytes);

    const bool StopCompiling =
        StopIfSameHash
        && (HashString == PreviousHashString)
        && (! Context.getSourceManager().getDiagnostics().hasErrorOccurred());

    if (TopLevelHashStream) {
//      if (!StopCompiling) //TODO: need to rewrite file everytime, gets cleared on open(): FIX THIS
          TopLevelHashStream->write(HashString.c_str(), HashString.length());
      delete TopLevelHashStream;
    }

    // After this binary finishes, we have to call link_object_file to
    // copy away our object file.
    std::string OutputFileCopy = OutputFile + ".hash_copy";
    objectfile_copy = strdup(OutputFileCopy.c_str());
    objectfile = strdup(OutputFile.c_str());
    atexit(link_object_file);

    // Sometimes we do terminal output
    if (Terminal) {
        *Terminal << "hash-start-time-ns "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(
                  StartHashing.time_since_epoch()).count() << "\n";
        *Terminal << "top-level-hash: " << HashString << "\n";
        *Terminal << "processed-bytes: " << ProcessedBytes << "\n";
        *Terminal << "parse-time-ns: "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(
                     StartHashing - StartCompilation).count() << "\n";
        *Terminal << "hash-time-ns: "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(
                     FinishHashing - StartHashing).count() << "\n";
        *Terminal << "element-hashes: [";
        for (const auto &SavedHash : Visitor.DeclSilo) {
            const Decl *D = SavedHash.first;
            const Hash::Digest &Dig = SavedHash.second;
            // Only Top-level declarations
            if (D->getDeclContext() &&
                isa<TranslationUnitDecl>(D->getDeclContext()) && isa<NamedDecl>(D)) {
                if (isa<FunctionDecl>(D))
                    *Terminal << "(\"function:";
                else if (isa<VarDecl>(D))
                    *Terminal << "(\"variable ";
                else if (isa<RecordDecl>(D))
                    *Terminal << "(\"record ";
                else
                    continue;

                *Terminal << cast<NamedDecl>(D)->getName();
                *Terminal << "\", \"";
                *Terminal << Dig.asString();
                *Terminal << "\"), ";
            }
        }
        *Terminal << "]\n";
    }


    if (StopCompiling) {
        struct stat dummy;
        // Object file not existing. Copy it from the backup
        bool copy_exists =
            (stat(OutputFileCopy.c_str(), &dummy) == 0);
        bool orig_exists =
            (stat(OutputFileCopy.c_str(), &dummy) == 0);
        if (copy_exists) {
            if (orig_exists) {
                unlink(OutputFile.c_str());
            }
            link(OutputFileCopy.c_str(), OutputFile.c_str());
            if (stat(OutputFile.c_str(), &dummy) == 0) {
                if (utime(OutputFile.c_str(), nullptr) == 0) {
                    // touch object file
                    if (Terminal) {
                        *Terminal << "skipped: true\n";
                    }
                    exit(0);
                }
            }
        }
    }
    if (Terminal) {
        *Terminal << "skipped: false\n";
    }
  }

private:
  // Returns true if the -stop-if-same-hash flag is set, else false.
  void hashCommandLineArguments() {
    // Get command line arguments
    const std::string PPID{std::to_string(getppid())};
    const std::string FilePath = "/proc/" + PPID + "/cmdline";
    std::ifstream CommandLine{FilePath};
    if (CommandLine.good()) { // TODO: move this to own method?
      std::list<std::string> CommandLineArgs;
      std::string Arg;
      do {
        getline(CommandLine, Arg, '\0');
        if ("-o" == Arg) {
          // throw away next parameter (name of outfile)
          getline(CommandLine, Arg, '\0');
          continue;
        }
        if (Arg.size() > 2 && Arg.compare(Arg.size() - 2, 2, ".c") == 0)
          continue; // don't hash source filename

        if (Arg.find("-stop-if-same-hash") != std::string::npos) {
            continue; // also don't hash this (plugin argument)
        }
        if (Arg.find("-hash-verbose") != std::string::npos) {
            continue; // also don't hash this (plugin argument)
        }

        CommandLineArgs.push_back(Arg);
      } while (Arg.size());

      Visitor.hashCommandLine(CommandLineArgs);
    } else {
      errs() << "Warning: could not open file \"" << FilePath
             << "\", cannot hash command line arguments.\n";
    }
  }

  raw_ostream *const TopLevelHashStream;
  raw_ostream *const Terminal;

  const std::string PreviousHashString;
  const std::string OutputFile;
  TranslationUnitHashVisitor Visitor;

  bool StopIfSameHash;

};

class HashTranslationUnitAction : public PluginASTAction {
protected:
    bool StopIfSameHash;
    bool Verbose;

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef) override {
    const std::string &OutputFile = CI.getFrontendOpts().OutputFile;
    const std::string HashFile = OutputFile + ".hash";
    const std::string PreviousHashString = getHashFromFile(HashFile);

    // Write hash database to .o.hash if the compiler produces a object file

    if ((CI.getFrontendOpts().ProgramAction == frontend::EmitObj
         || CI.getFrontendOpts().ProgramAction == frontend::EmitBC
         || CI.getFrontendOpts().ProgramAction == frontend::EmitLLVM)
        && OutputFile != "" && OutputFile != "/dev/null") {
        /* OK.Let's run */
    } else if  (CI.getFrontendOpts().ProgramAction == frontend::ParseSyntaxOnly) {
        /* OK Let's run */
    } else {
        return make_unique<ASTConsumer>();
    }

    raw_ostream *Out = nullptr;
    if (OutputFile != "" && OutputFile != "/dev/null") {
      std::error_code Error;
      Out = new raw_fd_ostream(HashFile, Error, sys::fs::F_Text); //TODO: this overrides/clears .hash file. currently rewriting file after check. FIX THIS!
      if (Error) {
        errs() << "Could not open ast-hash file: " << OutputFile << "\n";
      }
    }
    raw_ostream *Terminal = nullptr;
    if (Verbose) Terminal = &errs();
    return make_unique<HashTranslationUnitConsumer>(Out, Terminal,
                                                    PreviousHashString,
                                                    OutputFile,
                                                    StopIfSameHash);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &Args) override {
    StartCompilation = std::chrono::high_resolution_clock::now();
    Verbose = false;
    StopIfSameHash = false;
    for (const std::string &Arg : Args) {
        if (Arg == "-hash-verbose") {
            Verbose = true;
        }
        if (Arg == "-stop-if-same-hash") {
            StopIfSameHash = true;
        }
    }
    if (Args.size() && Args[0] == "help") {
      // FIXME
      PrintHelp(errs());
    }
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    return AddBeforeMainAction;
  }

  void PrintHelp(raw_ostream &Out) {
    Out << "Help for PrintFunctionNames plugin goes here\n";
  }

private:
  const std::string getHashFromFile(const std::string &FilePath) {
    std::string HashString;
    std::ifstream FileStream(FilePath);
    if (FileStream.good()) {
      getline(FileStream, HashString);
      if (Verbose) {
          errs() << FilePath << ": old hash string: " << HashString << "\n";
      }
    } else {
      if (Verbose) {
          errs() << "Warning: could not open file \"" << FilePath
                 << "\", cannot read previous hash.\n";
      }
    }
    return HashString;
  }

};


static FrontendPluginRegistry::Add<HashTranslationUnitAction>
    X("clang-hash", "hash translation unit");
