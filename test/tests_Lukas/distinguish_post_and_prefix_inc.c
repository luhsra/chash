

void f(){
	int a = 3;
	int b = a++; {{A}}
	int b = ++a; {{B}}
	
}
	
/*
 * check-name: distinguish post and prefix increment when necessary
 * obj-not-diff: optimization is ok
 * assert-ast: A != B
 * assert-obj: A != B
 */
//TODO: tests give obj A != B
