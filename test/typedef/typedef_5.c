typedef long used; {{A}}
typedef char used; {{B}}

used first;
/*
 * check-name: typedef diff
 * B != A
 */

