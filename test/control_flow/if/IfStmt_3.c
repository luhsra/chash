void a() {
  ;         {{A}}
  if (0) {} {{B}}
  if (1) {} {{C}}
}

/*
 * check-name: if block 3
 * obj-not-diff: blame optimisation
 * assert-ast: A != B, A != C, B != C
 * assert-obj: A == B, A == C, B == C
 */
