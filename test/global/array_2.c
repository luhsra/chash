char c[8]; {{A}}
char *c; {{B}}
char c[4]; {{C}}
/*
 * check-name: Pointer, array
 * B != A, C != B, A == C
 */

