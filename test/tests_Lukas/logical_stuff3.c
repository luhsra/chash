

void f(){
	int a = 3;
	int b = 2;
	do{ {{A}}
		a = 1; {{A}}
	} while(1); {{A}}

	while(1){ {{B}}
		a = 1; {{B}}
	} {{B}}


	if( 1){ 
		a = 2;
	}
	else{ {{C}}
		a = 3; {{C}}
	} {{C}}
}
	
/*
 * check-name: do while, if/ else
 * obj-not-diff: optimization is oki
 * A != B, B != C, A != C
 */


