void a(){
	int b;
	goto ende; {{B}}
	b = 0; {{A}}
	int c;
ende:	b = 0; {{B}}
}

/*
 * check-name: goto forwards to not empty label
 * A != B
 */
