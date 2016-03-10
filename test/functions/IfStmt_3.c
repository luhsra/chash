void a(){
	; {{A}}
	if(0){} {{B}}
	if(1){} {{C}}
}

/*
 * check-name: if mit leerem Block
 * A != B, B != C, C != A
 */
