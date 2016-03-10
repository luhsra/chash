void a(){
label1: ; {{A}}
label2:	; {{C}}
	; {{B}}
}


/*
 * check-name: label empty, not used
 * obj-not-diff: why should they?
 * A != B, B != C, c != A
 */
