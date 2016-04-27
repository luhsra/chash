void f2();

void f1(){
	f2(); {{A}}
	{{B}}
}

void f2(){
	f1();
	int a; {{C}}
}
/*
 * check-name: cross recursion
 * assert-obj: B != A, B != C, A != C
 */

