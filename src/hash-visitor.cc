#include "hash-visitor.h"

using namespace clang;
using namespace std;

string TranslationUnitHashVisitor::GetHash() {
    return "FIXME";
}

bool TranslationUnitHashVisitor::VisitVarDecl(VarDecl *Declaration) {
    // For debugging, dumping the AST nodes will show which nodes are already
    // being visited.
    Declaration->dump();

    // The return value indicates whether we want the visitation to proceed.
    // Return false to stop the traversal of the AST.
    return true;
}
