int a = 1;     {{A}}
int a = 1.444; {{B}}

/*
 * check-name: implicit cast
 * obj-not-diff: yes (constant folding)
 * assert-ast: A != B
 * assert-obj: A == B
 */
