int a = 1 ? 2 : 0; {{A}}
int a = 0 ? 2 : 0; {{B}}
int a = 1 ? 0 : 2; {{C}}
int a = 0 ? 0 : 2; {{D}}

/*
 * check-name: ConditionalOperator
 * obj-not-diff: A == D, B == C
 * assert-ast: A != D, B != C
 * assert-obj: A == D, B == C, A != B, A != C, B != D, C != D
 */
