void a(){
	{{A}}
	return; {{B}}
}

/*
 * check-name: ReturnStatement 1
 * obj-not-diff: true
 * A != B
 */
