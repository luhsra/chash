typeof (int) x; {{A}}
int x; {{B}}
/*
 * check-name: WTF
 * obj-not-diff: y
 * assert-ast: A != B
 * assert-obj: A == B
 */
