char c;    {{A}}
char *c;   {{B}}
char c[4]; {{C}}

/*
 * check-name: char vs pointer vs array
 * assert-obj: A != B, A != C, B != C
 */

