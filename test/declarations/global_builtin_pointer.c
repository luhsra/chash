int *first;          {{A}}
int *first;          {{C}}
unsigned int *first; {{B}}

{{A:char *}}{{B:long *}}{{C:char *}} second;

typedef int unused; {{C}}

/*
 * check-name: Definition of global variable with Pointer 2
 * obj-not-diff: yes
 * assert-ast: A != B, B != C, A == C
 * assert-obj: A == B, B == C
 */
