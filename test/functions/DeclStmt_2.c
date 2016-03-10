void a(){
	int b; {{A}}
	int b = 0; {{B}}
}

/*
 * check-name: initialisation
 * A != B
 */
