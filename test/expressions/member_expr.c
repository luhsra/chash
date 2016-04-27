
struct a{
	int c;
	int *b;
};

void func(void){

	struct a *d;
	int e = d->c;

}


/*
 * check-name: NOT, L_NOT
 * A != B, A != C, B != C
 */
//TODO: gar keine testcases!
