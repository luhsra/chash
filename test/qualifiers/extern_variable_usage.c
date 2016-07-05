extern int ext; {{A}}
extern char ext; {{B}}

void foo() {
    ext++;
}
/*
 * check-name: extern variable usage
 * assert-ast: A != B
 */
