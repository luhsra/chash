typedef int * used;

used first; {{A}}
int *first; {{B}}
/*
 * check-name: typedef int *
 * B == A
 */

