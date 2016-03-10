int a(){
	int b = 0;
	return 0; {{A}}
	return b; {{B}}
}

/*
 * check-name: non-empty return
 * A != B
 */
