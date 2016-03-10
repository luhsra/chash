void a(){
	; {{A}}
	if(0){} {{B}}
}

/*
 * check-name: if mit Bedingung false und leerem Block
 * obj-not-diff: blame optimisation
 * A != B
 */
