void a(){
	; {{A}}
	while(0){} {{B}}
	while(1){} {{C}}
}

/*
 * check-name: while with empty body
 * A != B, B != C, C != A
 */
