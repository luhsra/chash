void a(){
	int b;
	b = 0; {{A}}
label1:	b = 0; {{B}}
label2: b = 0; {{C}}
}

/*
 *check-name: label not empty, not used
 * obj-not-diff: why should they?
 * A != B, A != C, B != C
 */
