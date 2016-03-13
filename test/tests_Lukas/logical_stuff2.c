

void f(){
	int a = 2;
	int b = 1;
	if( a && b){ {{A}}
	if( a || b){ {{B}}
	if( a & b){ {{C}}
		a = 3;
	}
}
	
/*
 * check-name: logical operators
 * A != B, A != C, B != C
 */


