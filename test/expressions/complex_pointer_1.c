void f(void) {
  char a;       {{A}}
  const char a; {{B}}
  const char b;
  char* c = &a;
  c = &b;
  return;
}

/*
 * check-name: complex pointer (maybe our problem)
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
//TODO: besseren Testcase bauen!
