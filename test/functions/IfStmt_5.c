void a(){
	if(1){}
	; {{A}}
	else{} {{B}}
}

/*
 * check-name: else block 2
 * obj-not-diff: blame optimisation
 * A != B
 */
