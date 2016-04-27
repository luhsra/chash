int first; {{A}}
volatile int first; {{B}}

/*
 * check-name: Testing volatile
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
