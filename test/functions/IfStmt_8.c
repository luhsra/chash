void a(){
	int b = 0;
	if(0){}
	; {{A}}
	else{ b++; } {{B}}
}

/*
 * check-name: else block non-empty 1
 * A != B
 */
