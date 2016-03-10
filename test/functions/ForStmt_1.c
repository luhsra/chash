void a(){
	; {{A}}
	for(int b = 0; b < 100; b++){} {{B}}
	for(int b = 100; b < 100; b++){} {{C}}
}

/*
 * check-name: for with empty body
 * A != B; B != C; C != A
 */
