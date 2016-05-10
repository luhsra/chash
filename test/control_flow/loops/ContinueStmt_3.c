void a() {
  do {} while (0);            {{A}}
  do { continue; } while (0); {{B}}
}

/*
 * check-name: continue in do-while (0)
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
