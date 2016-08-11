
void foo()
{
    int ptr = 0;
    if (0) {
        ptr = 1; {{A}}
    }
    ptr = 1; {{B}}
}


/*
 * check-name: BUG
 * assert-ast: A != B
 */
