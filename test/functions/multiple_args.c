void func(int a, ...) {} {{A}}
void func(int a) {}     {{B}}

/*
 * check-name: multiple_args
 * assert-ast: A != B
 * obj-not-diff: On Linux(obj files different) and FreeBSD (objfiles equal)
 */
