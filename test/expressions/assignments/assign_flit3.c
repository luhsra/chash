double a = 1.0;  {{A}}
double a = 1.0f; {{B}}
double a = 1.0F; {{C}}

/*
 * check-name: same value but different float literals
 * obj-not-diff: because float literals have to be cast
 * assert-ast: A != B, A != C, B == C
 * assert-obj: A == B, A == C
 */
