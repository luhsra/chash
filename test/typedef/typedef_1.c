typedef int used;

used first; {{A}}
int first; {{B}}
/*
 * check-name: Typedef int
 * assert-obj: A == B
 */

