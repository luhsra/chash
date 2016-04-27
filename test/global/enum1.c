enum my_enum{aa,bb,cc}; {{A}}
enum my_enum{bb,aa,cc}; {{B}}

void func(void){
	int karte = aa;
}

/*
 * check-name: Initlist array
 * obj-not-diff: might have 
 * assert-ast: A != B
 * assert-obj: A != B
 */
