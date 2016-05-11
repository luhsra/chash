#include <stdio.h>  {{A}}
#include <stdlib.h> {{B}}
{{C}}

/*
 * check-name: not used dynamic include
 * assert-ast: A == B, A == C, B == C
 */
