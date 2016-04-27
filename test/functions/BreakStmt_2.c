void a(){
	while(1){} {{A}}
	while(1){ break; } {{B}}
}

/*
 * check-name: break in while 2
 * assert-obj: A != B
 */
