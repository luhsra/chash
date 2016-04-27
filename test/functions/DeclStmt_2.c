void a(){
	int b; {{A}}
	int b = 0; {{B}}
}

/*
 * check-name: initialisation
 * assert-obj: A != B
 */
