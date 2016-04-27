int **first; {{A}}
int *first; {{B}}
int first; {{C}}

/*
 * check-name: Definition of Global Variable with Pointer 2
 * obj-not-diff: yes
 * assert-ast: A != B, A != C, B != C
 * assert-obj: A == B, A != C, B != C
 * A == B because both are ptr, != C because C is not
 */
