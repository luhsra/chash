void a() {
    do {} while (0);         {{A}}
	do { break; } while (0); {{B}}
}

/*
 * check-name: break in do-while(0)
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
