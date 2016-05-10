void a() {
  ;         {{A}}
  if (0) {} {{B}}
  if (1) {} {{C}}
}

/*
 * check-name: if block 3
 * obj-not-diff: blame optimisation
 * assert-ast: A != B, B != C, C != A
 * assert-obj: A == B, B == C, C == A
 */
