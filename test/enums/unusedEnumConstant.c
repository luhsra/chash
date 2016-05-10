enum {aa, bb, cc};     {{A}}
enum {aa, bb, cc, dd}; {{B}}

int a = cc;

/*
 * check-name: Adding unused enum constant
 * assert-ast: A == B
 */
