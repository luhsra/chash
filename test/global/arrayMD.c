int inty[4][2];	{{A}}
int inty[2][4]; {{B}}

/*
 * check-name: Initlist array
 * obj-not-diff: might have 
 * assert-ast: A != B
 * assert-obj: A == B
 */
