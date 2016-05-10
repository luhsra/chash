int first;       {{A}}
int first;       {{C}}
const int first; {{B}}

const int second; {{A}}
int second;       {{C}}

/*
 * check-name: const vs non-const variable declarations (3)
 * obj-not-diff: A == C
 * assert-ast: A != C
 * assert-obj: A == C, A != B, B != C
 */
