int first; {{A}}
int first; {{C}}
const int first; {{B}}

int wombat; {{C}}
const int wombat; {{A}}
/*
 * check-name: Testing const then without L
 * obj-not-diff: yes
 * assert-ast: A != B, A != C, B != C
 * assert-obj: A != B, A == C, B != C
 */
