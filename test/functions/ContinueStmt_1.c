void a(){
	while(0){} {{A}}
	while(0){ continue; } {{B}}
}

/*
 * check-name: continue im while 1
 * A != B
 */
