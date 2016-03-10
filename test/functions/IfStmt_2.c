void a(){
	; {{A}}
	if(1){} {{B}}
}

/*
 * check-name: if mit Bedingung wahr und leerem Block
 * obj-not-diff: blame optimisation
 * A != B
 */
