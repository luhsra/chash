void a() {
  int b = 0;
  ;                      {{A}}
  do { b++; } while (0); {{B}}
  do { b++; } while (1); {{C}}
}

/*
 * check-name: do-while with non-empty body
 * assert-obj: A != B, A != C, B != C
 */
