void a(){
	{{A}}
	return; {{B}}
}

/*
 * check-name: empty return
 * obj-not-diff: true
 * assert-ast: A != B
 * assert-obj: A == B
 */
