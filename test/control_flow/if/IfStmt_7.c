void a() {
  int b = 0;
  ;               {{A}}
  if (1) { b++; } {{B}}
}

/*
 * check-name: if block non-empty 2
 * assert-obj: A != B
 */
