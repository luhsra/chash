void a(){
	int b = 0;
	int i = 0; {{B}}
	int i = 0; {{C}}
	int c = 0;
	for({{A: int i = 0}} {{B: b}} {{C:  }}; i < 100; i++){}
}

/*
 * check-name: init-stmt in for
 * A != B; B != C; C != A
 */
