int a = 04711;

void f(){
	int b = -a; {{A}}
	int b = a; {{B}}
}
/*
 * check-name: unminus
 * B != A
 */
