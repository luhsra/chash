int a = 0;

void f(){
	int b = a; {{A}}
	int b = !a; {{B}}
	int b = ~a; {{C}}
}
/*
 * check-name: NOT, L_NOT
 * assert-obj: A != B, A != C, B != C
 */
