typedef const int used;

used first;      {{A}}
int first;       {{B}}
const int first; {{C}}

/*
 * check-name: typedef const
 * obj-not-diff: yes
 * assert-ast: A != B, A != C, B != C
 * assert-obj: A == B, A == C, B == C
 */
