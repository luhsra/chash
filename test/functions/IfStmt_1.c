void a(){
	; {{A}}
	if(0){} {{B}}
}

/*
 * check-name: if block 1
 * obj-not-diff: blame optimisation
 * A != B
 */
