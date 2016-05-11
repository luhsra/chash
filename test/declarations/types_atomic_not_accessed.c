_Atomic double a; {{A}}
double a;         {{B}}

/*
 * check-name: atomic types (not accessed)
 * obj-not-diff: maybe
 * assert-ast: A != B
 * assert-obj: A == B
 */
