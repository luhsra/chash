typeof (int) x; {{A}}
int x; {{B}}
/*
 * check-name: WTF
 * obj-not-diff: y
 * B != A
 */

