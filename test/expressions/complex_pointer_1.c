void f(void) {
  char a{{B:[1]}};
  char* c = {{A:&}}a;
  char b = 0;
  c = &b;
  return;
}

/*
 * check-name: complex pointer (maybe our problem)
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
