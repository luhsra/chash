int first; {{A}}
const int first; {{B}}

char second;
/*
 * check-name: Testing const
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
