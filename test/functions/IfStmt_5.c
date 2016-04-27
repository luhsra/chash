void a(){
	if(1){}
	; {{A}}
	else{} {{B}}
}

/*
 * check-name: else block 2
 * obj-not-diff: blame optimisation
 * assert-ast: A != B
 * assert-obj: A == B
 */
