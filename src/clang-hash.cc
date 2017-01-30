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
#include <fcntl.h>



using namespace clang;
using namespace llvm;

static std::chrono::high_resolution_clock::time_point StartCompilation;

static enum {
    ATEXIT_NOP,
    ATEXIT_FROM_CACHE,
    ATEXIT_TO_CACHE,
} atexit_mode = ATEXIT_NOP;

static char *hashfile = NULL;

static char *hash_new = NULL;
static char *hash_old = NULL;

static char *objectfile = NULL;
static char *objectfile_copy = NULL;

static void link_object_file() {
    if (atexit_mode == ATEXIT_NOP) {
        return;
    }
    assert(objectfile != nullptr);
    assert(objectfile_copy != nullptr);

    char *src = nullptr, *dst = nullptr;

    if (atexit_mode == ATEXIT_FROM_CACHE) {
        src = objectfile_copy;
        dst = objectfile;
    } else {
        src = objectfile;
        dst = objectfile_copy;
    }

    // Record Events
    if (getenv("CLANG_HASH_LOGFILE")) {
        int fd = open(getenv("CLANG_HASH_LOGFILE"),
                      O_APPEND | O_WRONLY | O_CREAT,
                      0644);
        write(fd, atexit_mode == ATEXIT_FROM_CACHE ? "H" : "M", 1);
        close(fd);
    }

    /* If destination exists, we have to unlink it. */
    struct stat dummy;
    if (stat(dst, &dummy) == 0) { // exists
        if (unlink(dst) != 0) { // unlink failed
            perror("clang-hash: unlink objectfile/objectfile copy");
            return;
        }
    }
    if (stat(src, &dummy) != 0) { // src exists
        perror("clang-hash: source objectfile/objectfile copy does not exist");
        return;
    }

    // Copy by hardlink
    if (link(src, dst) != 0) {
        perror("clang-hash: objectfile update failed");
        return;
    }

    // Write Hash to hashfile
    if (atexit_mode == ATEXIT_TO_CACHE) {
        FILE *f = fopen(hashfile, "w+");
        fwrite(hash_new, strlen(hash_new), 1, f);
        fclose(f);
    } else if (atexit_mode == ATEXIT_FROM_CACHE) {
        // Update Timestamp
        utime(dst, NULL);
    }
}

class HashTranslationUnitConsumer : public ASTConsumer {
public:
   HashTranslationUnitConsumer(raw_ostream *OS, bool StopIfSameHash)
      : Terminal(OS), StopIfSameHash(StopIfSameHash) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) override {
    /// Step 1: Calculate Hash
    const auto StartHashing = std::chrono::high_resolution_clock::now();

    // Traversing the translation unit decl via a RecursiveASTVisitor
    // will visit all nodes in the AST.
    Visitor.hashDecl(Context.getTranslationUnitDecl());

    hashCommandLineArguments();

    const auto FinishHashing = std::chrono::high_resolution_clock::now();

    unsigned ProcessedBytes;
    const std::string HashString = Visitor.getHash(&ProcessedBytes);
    hash_new = strdup(HashString.c_str());

    // Step 2: Consequent Handling
    const bool HashEqual =
        (hash_old != nullptr) // Cache Valid?
        && (std::string(hash_old) == HashString)
        && (! Context.getSourceManager().getDiagnostics().hasErrorOccurred());
    // Step 2.1: -hash-verbose
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
            break;
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
        *Terminal << "hash-equal:" << HashEqual << "\n";
        *Terminal << "skipped:" << (HashEqual && StopIfSameHash) << "\n";
    }

    if (StopIfSameHash && objectfile != nullptr) {
        // We are in caching mode and there should be an objectfile
        atexit(link_object_file);
        if (HashEqual) {
            atexit_mode = ATEXIT_FROM_CACHE;
            exit(0);
        } else {
            atexit_mode = ATEXIT_TO_CACHE;
            // Continue with compilation
        }
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

  raw_ostream *const Terminal;
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
    if (OutputFile != "" && OutputFile != "/dev/null") {
        std::string tmp;
        objectfile = strdup(OutputFile.c_str());

        tmp = OutputFile + ".hash";
        hashfile = strdup(tmp.c_str());

        tmp = OutputFile + ".hash.copy";
        objectfile_copy = strdup(tmp.c_str());
    }

    // Retrieve Hash and check if copy is intact
    const std::string PreviousHashString = getHashFromFile(hashfile ? hashfile : "");
    if (PreviousHashString != "") {
        struct stat dummy;
        if (stat(objectfile_copy, &dummy) == 0) {
            hash_old = strdup(PreviousHashString.c_str());
        }
    }

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

    raw_ostream *Terminal = nullptr;
    if (Verbose) Terminal = &errs();

    return make_unique<HashTranslationUnitConsumer>(Terminal, StopIfSameHash);
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
