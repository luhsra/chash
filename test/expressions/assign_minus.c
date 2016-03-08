int a = 1 - 1; {{A}}
int a = 0; {{B}} 
/*
 * check-name: minus
 * obj-not-diff: maybe
 * B != A 
 */
