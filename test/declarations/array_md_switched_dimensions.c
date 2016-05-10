int inty[4][2];	{{A}}
int inty[2][4]; {{B}}

/*
 * check-name: array switched dimensions
 * obj-not-diff: looks the same in memory
 * assert-ast: A != B
 * assert-obj: A == B
 */
