void a(){
	while(0){} {{A}}
	while(0){ continue; } {{B}}
}

/*
 * check-name: continue in while 1
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
