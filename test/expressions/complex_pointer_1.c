void f(void){

	char a;
	const char b;
	char * c = &a;
	c = &b;

	return;
}

{{A}}

/*
 * check-name: complex pointer (maybe our problem)
 */
//TODO: testcases fehlen
