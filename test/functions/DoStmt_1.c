void a(){
	do {} while(0); {{A}}
	do {} while(1); {{B}}
	; {{C}}
}

/*
 * check-name: do-while with empty body
 * A != B; A != C; B != C
 */
