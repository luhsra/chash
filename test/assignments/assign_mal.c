int a = 1 * 1; {{A}}
int a = 1;     {{B}}

/*
 * check-name: mal
 * obj-not-diff: yes (constant folding)
 * assert-ast: A != B
 * assert-obj: A == B
 */
