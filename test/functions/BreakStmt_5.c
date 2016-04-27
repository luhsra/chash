void a(){
	for(int i = 0; i < 100; i++){} {{A}}
	for(int i = 0; i < 100; i++){ break; } {{B}}
}

/*
 * check-name: break in for
 * assert-obj: A != B
 */
