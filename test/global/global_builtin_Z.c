int first = 0; {{A}}
int first = 1; {{C}}
unsigned int first = 0; {{B}}

{{A:char}}{{B:long}}{{C:char}} second;

typedef int unused; {{C}}
/*
 * check-name: Definition of Global Variable with Assignment
 * A != C, B != A
 */
