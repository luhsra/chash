typedef int used;

used first; {{A}}
int first; {{B}}
/*
 * check-name: Typedef int
 * B == A
 */

