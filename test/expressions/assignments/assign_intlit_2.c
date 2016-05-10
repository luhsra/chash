int i = 4711;  {{A}}
int i = 4711l; {{B}}
int i = 4711L; {{C}}

/*
 * check-name: same value but different integer literal 2 (int vs long)
 * obj-not-diff: because 4711 is int
 * assert-ast: A != B, A != C, B == C
 * assert-obj: A == B, A == C
 */

