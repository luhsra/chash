void a(){
	do {} while(0); {{A}}
	do { break; } while (0); {{B}}
}

/*
 * check-name: break in do-while 1
 * obj-not-diff: ich weiss nicht, warum
 * A != B
 */
