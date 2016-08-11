
void foo()
{
    int ptr = 0;
    if (0) {
        ptr = 1; {{A}}
    }
    ptr = 1; {{B}}
}


/*
 * check-name: Assignment before/after block end (CompoundStmt)
 * assert-ast: A != B
 */
