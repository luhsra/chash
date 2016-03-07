int first; {{A}}
int first; {{C}}
unsigned int first; {{B}}

{{A:char}}{{B:long}}{{C:char}}{{D:char *}} second;

typedef int unused; {{C}}
/*
 * check-name: Definition of Global Variable
 * A == C, B != A
 */

unsigned int third; {{D}}
