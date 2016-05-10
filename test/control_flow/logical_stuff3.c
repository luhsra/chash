void f() {
  int a = 3;
  int b = 2;
  do {         {{A}}
    a = 1;     {{A}}
  } while (1); {{A}}

  while (1) { {{B}}
    a = 1;    {{B}}
  }           {{B}}

  if (1) {
    a = 2;
  }
  else {   {{C}}
    a = 3; {{C}}
  }        {{C}}

  {{D}}
}

/*
 * check-name: do/while vs while vs if vs if/else
 * obj-not-diff: optimization is ok
 * assert-ast: C != D
 * assert-obj: C == D, A != B, A != C, A != D, B != C, B != D
 */

