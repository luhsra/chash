void a(){
	while(1){} {{A}}
	while(1){ continue; } {{B}}
}

/*
 * check-name: continue in while 2
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
