int first;       {{A}}
const int first; {{B}}

int second;
const int third;

/*
 * check-name: const vs. non-const variable declaration (2)
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
