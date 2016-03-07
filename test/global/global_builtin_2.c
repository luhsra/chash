int first; {{A}}
int first; {{C}}
unsigned int first; {{B}}

{{A:char}}{{B:long}}{{C:char}}{{D:char *}} second;

typedef int unused; {{C}}

unsigned int third; {{D}}

/*
 * check-name: Definition of Global Variable with Pointer 1
 * A == C, B != A, C != D, A != D, B != D
 */
