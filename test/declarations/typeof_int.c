typeof (int) x; {{A}}
int x;          {{B}}

/*
 * check-name: declaration type: int vs typeof(int)
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
