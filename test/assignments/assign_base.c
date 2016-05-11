int a = 04711;  {{A}}
int a = 4711;   {{B}}
int a = 0x4711; {{C}}

/*
 * check-name: assignment: oct vs dec vs hex
 * assert-obj: A != B, A != C, B != C
 */
