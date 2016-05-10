int first;          {{A}}
int first;          {{C}}
unsigned int first; {{B}}

{{A:char}}{{B:long}}{{C:char}} second;

typedef int unused; {{C}}

/*
 * check-name: definition of global variables 1
 * assert-ast: A == C
 * assert-obj: A != B, B != C
 */
