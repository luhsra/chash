int i;
typeof (i) x; {{A}}
int x; {{B}}
/*
 * check-name: typeof 1
 * obj-not-diff: y
 * B != A
 */

