typedef int * used;

used first; {{A}}
int *first; {{B}}

/*
 * check-name: typedef int *
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
