int a = 0;

void f() {
  int b = a;  {{A}}
  int b = !a; {{B}}
  int b = ~a; {{C}}
}

/*
 * check-name: binary not vs logical not
 * assert-obj: A != B, A != C, B != C
 */
