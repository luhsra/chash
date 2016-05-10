void f() {
  int a = 3;
  int b = a++; {{A}}
  int b = ++a; {{B}}
}
	
/*
 * check-name: distinguish post and prefix increment when necessary
 * assert-obj: A != B
 */

