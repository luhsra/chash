void a(){
	int b = 0;
	if(0){ --b; }
	else{} {{A}}
	else{ b++; } {{B}}
}

/*
 * check-name: if & else block non-empty 1
 * assert-obj: A != B
 */
