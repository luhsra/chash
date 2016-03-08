int a = 04711;
int b = -a; {{A}}
int b = a; {{B}}

/*
 * check-name: unminus
 * B != A
 */
