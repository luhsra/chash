int a = 04711;

void f() {
  int b = -a; {{A}}
  int b =  a; {{B}}
}

/*
 * check-name: pos vs neg value
 * assert-obj: A != B
 */
