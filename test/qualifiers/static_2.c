void f(){

	static int a; {{A}}
	int a; {{B}}
// pay attention wheter obj-file is even created:
// the next line enforces this!
	a = 3;
}


/*
 * check-name: Testing static (in function)
 * obj-not-diff: yes //TODO rly?
 * assert-ast: A != B
 * assert-obj: A != B
 */
//TODO: test sagt obj A != B, obj-not-diff-comment sagt ==
