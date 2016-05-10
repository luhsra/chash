int first;          {{A}}
int first;          {{C}}
unsigned int first; {{B}}

{{A:char}}{{B:long}}{{C:char}}{{D:char *}} second;

typedef int unused; {{C}}

unsigned int third; {{D}}

/*
 * check-name: definition of global variables with pointer 1
 * assert-ast: A == C
 * assert-obj: A != B, A != D, B != C, B != D, C != D
 */
