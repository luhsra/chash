
{{A:char}}{{B:long}}second;


int callee(int); {{A}}
int callee(int); {{B}}

void caller(void){ {{A}}
void caller(void){ {{B}}

	int a = 8; {{A}}
	int a = 8; {{B}}
	callee(a); {{A}}
	callee(a); {{B}}
} {{A}}
} {{B}}

int callee(int a){ {{A}}
int callee(int a){ {{B}}
	return a + 1; {{A}}
	return a - 1; {{B}}
} {{A}}
} {{B}}


int main(){ {{A}}
int main(){ {{B}}
	
	caller(); {{A}}
	caller(); {{B}}

} {{A}}
} {{B}}


/*
 * check-name: Function-Test-1
 * B != A
 */

