

char** op(){
	char hi[4];

	int a = 8; {{A}}
	int a = 9; {{B}}
	int b = 19;

	a *= 2;
	b /= 4;
	a = a + b; {{A}}
	a = a + 8;
	a = a * b; {{B}}
	a++;

	char** ret = &hi;

	return ret;
}

int main(){

	int arg;
	//contains operations like + * - /
	op();
}
/*
 * check-name: Function-Test-2
 * assert-obj: B != A
 */


