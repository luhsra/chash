int a = 0;
int b = a; {{A}}
int b = !a; {{B}}
int b = ~a; {{C}}
/*
 * check-name: NOT, L_NOT
 * B != A, B != C, C != A 
 */
