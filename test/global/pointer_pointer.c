int **first; {{A}}
int *first; {{B}}
int first; {{C}}

/*
 * check-name: Definition of Global Variable with Pointer 2
 * obj-not-diff: yes
 * B != A, C != A, B != C
 */
