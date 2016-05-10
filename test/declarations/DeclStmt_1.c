void a() {
  ;      {{A}}
  int a; {{B}}
}

/*
 * check-name: DeclStmt 1
 * obj-not-diff: blame optimisation
 * assert-ast: A != B
 * assert-obj: A == B
 */
