char c[8]; {{A}}
char *c;   {{B}}
char c[4]; {{C}}

/*
 * check-name: pointer vs arrays
 * assert-obj: A != B, A != C, B != C
 */
