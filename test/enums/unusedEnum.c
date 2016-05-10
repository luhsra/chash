enum {aa, bb, cc}; {{A}}
{{B}}

int j;

/*
 * check-name: Definition of unused enum
 * assert-ast: A == B
 */
