void a(){
	for(int i = 0; i < 100; i++){} {{A}}
	for(int i = 0; i < 100; i++){ continue; } {{B}}
}

/*
 * check-name: continue in for
 * obj-not-diff: yes
 * A != B
 */
