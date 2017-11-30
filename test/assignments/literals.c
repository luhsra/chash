char* a = "Foo"; {{A}}
char* a = "Foo\0Bar"; {{B}}
char* a = "Foo\0Bax"; {{C}}

/*
 * check-name: Different String literals
 * assert-obj: A != B, B != C
 */
