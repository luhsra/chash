void a() {
  int b = 0;
  ;                                        {{A}}
  for (int i = 0; i < 100; i++) { b++; }   {{B}}
  for (int i = 100; i < 100; i++) { b++; } {{C}}
}

/*
 * check-name: for with non-empty body
 * assert-obj: A != B, A != C, B != C
 */
