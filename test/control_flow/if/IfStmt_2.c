void a() {
  ;         {{A}}
  if (1) {} {{B}}
}

/*
 * check-name: if block 2
 * obj-not-diff: blame optimisation
 * assert-ast: A != B
 * assert-obj: A == B
 */
