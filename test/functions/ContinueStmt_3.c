void a(){
	do {} while(0); {{A}}
	do { continue; } while(1); {{B}}
}

/*
 * check-name: continue in do-while 1
 * A != B
 */
