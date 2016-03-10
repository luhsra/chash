void a(){
	{{A}}
	return; {{B}}
}

/*
 * check-name: empty return
 * obj-not-diff: true
 * A != B
 */
