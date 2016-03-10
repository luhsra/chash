void a(){
	if(0){}
	; {{A}}
	else{} {{B}}
}

/*
 * check-name: else block 1
 * obj-not-diff: blame optimisation
 * A != B
 */
