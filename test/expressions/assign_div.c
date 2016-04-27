int a = 1 / 1; {{A}}
int a = 1; {{B}} 
/*
 * check-name: divide
 * obj-not-diff: maybe
 * assert-ast: A != B
 * assert-obj: A == B
 */
