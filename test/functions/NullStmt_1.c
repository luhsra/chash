void a(){
	{{A}}
	; {{B}}
}

/*
 * check-name: NullStatement 1
 * obj-not-diff: of course
 * assert-ast: A != B
 * assert-obj: A == B
 */
