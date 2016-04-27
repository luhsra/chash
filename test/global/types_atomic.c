_Atomic double a; {{A}}
double a; {{B}}
/*
 * check-name: Atomic Types
 * obj-not-diff: vll.
 * assert-ast: A != B
 * assert-obj: A == B
 */
//TODO: obj war != ok so?
