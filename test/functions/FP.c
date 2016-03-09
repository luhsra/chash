void (*funky)(int); {{A}}
void (*funky)(int *); {{B}}
/*
 * check-name: FP
 * obj-not-diff: ?
 * B != A
 */

