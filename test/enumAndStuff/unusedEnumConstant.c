enum{aa, bb, cc}; {{A}}
enum{aa, bb, cc, dd}; {{B}}

int a = cc;
/*
 * check-name: Adding unused enum constatnt
 * assert-ast: A == B
 */
