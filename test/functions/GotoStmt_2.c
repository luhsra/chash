void a(){
ende:	;
	; {{A}}
	goto ende; {{B}}
}

/*
 * check-name: Goto backwards
 * A != B
 */
