int a = 1;
int b = a; {{A}}
int b = 1; {{B}}

/*
 * check-name: declref
 * obj-not-diff: maybe
 * B != A
 */
