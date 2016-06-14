typedef const int used;

used first;      {{A}}
int first;       {{B}}
const int first; {{C}}

/*
 * check-name: typedef const
 * obj-not-diff: yes
 * assert-ast: A != B, B != C, A == C
 * assert-obj: A == B, B == C
 */
