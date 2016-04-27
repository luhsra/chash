int a = 1;

void f(){
	int b = a; {{A}}
	int b = 1; {{B}}
}

/*
 * check-name: declref
 * assert-obj: A != B
 */
