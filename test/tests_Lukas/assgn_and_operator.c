

void f(){
	int a = 2;
	int b = 1;
	b += a; {{A}}
	b -= a; {{B}}
	b *= a; {{C}}
	b /= a; {{D}}

}
/*
 * check-name: assgn_and_operator
 * assert-obj: A != C, A != D, B != C, B != D, C !=D
 */


