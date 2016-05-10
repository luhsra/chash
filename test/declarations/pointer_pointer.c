int **first; {{A}}
int *first;  {{B}}
int first;   {{C}}

/*
 * check-name: Definition of Global Variable with Pointer (int vs int* vs int**)
 * obj-not-diff: A == B because both are ptr; != C because C is not
 * assert-ast: A != B
 * assert-obj: A == B, A != C, B != C
 */
