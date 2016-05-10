void (*funky)(int);   {{A}}
void (*funky)(int *); {{B}}

/*
 * check-name: function pointer
 * obj-not-diff: ? seems to be the same
 * assert-ast: A != B
 * assert-obj: A == B
 */
