void a(){
	int b = 0;
	if(1){ --b; }
	else{}; {{A}}
	else{ b++; } {{B}}
}

/*
 * check-name: if & else block non-empty 2
 * obj-not-diff: yes
 * A != B
 */
