void a() {
  int b = 0;
  ;               {{A}}
  if (0) { b++; } {{B}}
}

/*
 * check-name: if block non-empty 1
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
