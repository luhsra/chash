void a(){
	int b;
	goto ende; {{B}}
	b = 0; {{A}}
ende:	b = 0; {{B}}
}

/*
 * check-name: goto forwards to not empty label
 * obj-not-diff: blame optimisation
 * A != B
 */
