int a = 1; {{A}}
int a = (int) 1.444; {{B}}

/*
 * check-name: cast
 * obj-not-diff: maybe
 * assert-ast: A != B
 * assert-obj: A == B
 */
