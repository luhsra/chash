#include <stdio.h> {{A}}

int variable;
{{B}}

/*
 * check-name: used Decls in include
 * assert-obj: B != A
 */
//TODO: was genau sollte das testen?
