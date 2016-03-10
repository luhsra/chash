void a(){
	; {{A}}
	if(0){} {{B}}
	if(1){} {{C}}
}

/*
 * check-name: if block 3
 * obj-not-diff: blame optimisation
 * A != B, B != C, C != A
 */
