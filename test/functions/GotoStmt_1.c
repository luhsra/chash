void a(){
	goto ende; {{B}}
	; {{A}}
ende:	; {{B}}
}

/*
 * check-name: Goto forwards
 * assert-obj: A != B
 */
