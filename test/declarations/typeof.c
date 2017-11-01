int i;
typeof (i) x;   {{A}}
int x;          {{B}}
typeof (2+3) x; {{C}}

/*
 * check-name: typeof (int vs typeof(i); i is int var)
 * obj-not-diff: yes
 * assert-ast: A != B, B != C, A != C
 * assert-obj: A == B, B == C
 */
