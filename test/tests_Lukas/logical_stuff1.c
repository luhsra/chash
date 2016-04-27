

void f(){
	int a = 2;
	int b = 1;
	if(1){ {{B}}
	while(1){ {{A}}
		a = 3;
	}
}
/*
 * check-name: difference between while and if
 * assert-obj: A != B
 */


