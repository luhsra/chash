void a(){
	int b = 0;
	if(1){}
	; {{A}}
	else{ b++; } {{B}}
}

/*
 * check-name: else block non-empty 2
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
