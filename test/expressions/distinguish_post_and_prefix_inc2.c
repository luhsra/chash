void f() {
  int a = 3;
  a++; {{A}}
  ++a; {{B}}
  int b = a;
}
	
/*
 * check-name: should not distinguish, when not necessary
 * obj-not-diff: object file isn't changed indeed!
 * assert-ast: A != B
 * assert-obj: A == B
 */
