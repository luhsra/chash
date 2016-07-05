extern int ext(void); {{A}}
extern char ext(void); {{B}}

void foo() {
    ext();
}

/*
 * check-name: extern function usage
 * assert-ast: A != B
 */
