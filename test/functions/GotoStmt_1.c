void a(){
	goto ende; {{B}}
	; {{A}}
ende:	; {{B}}
}

/*
 * check-name: Goto forwards
 * A != B
 */
