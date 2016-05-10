typedef long used; {{A}}
typedef char used; {{B}}

used first;

/*
 * check-name: typedef diff
 * assert-obj: A != B
 */

