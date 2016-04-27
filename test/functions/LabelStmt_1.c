void a(){
label1: ; {{A}}
label2:	; {{C}}
	; {{B}}
}


/*
 * check-name: label empty, not used
 * obj-not-diff: why should they? //TODO!
 * assert-obj: A != B, A == C, B != C
 */
//TODO: test sagt A != B und B != C
