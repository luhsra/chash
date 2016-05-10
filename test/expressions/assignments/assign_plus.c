int a = 1 + 1; {{A}}
int a = 2;     {{B}}

/*
 * check-name: plus
 * obj-not-diff: yes (constant folding)
 * assert-ast: A != B
 * assert-obj: A == B
 */
