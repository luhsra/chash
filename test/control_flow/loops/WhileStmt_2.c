void a() {
  int b = 0;
  while (0) { b++; } {{A}}
  while (1) { b++; } {{B}}
}

/*
 * check-name: while with non-empty body
 * assert-obj: A != B
 */
