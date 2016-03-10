void a(){
	do {} while(1); {{A}}
	do { continue; } while(1); {{B}}
}

/*
 * check-name: continue in do-while 2
 * obj-not-diff: optimisation
 * A != B
 */
