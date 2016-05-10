int first;       {{A}}
const int first; {{B}}

char second;

/*
 * check-name: const vs. non-const variable declaration (1)
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
