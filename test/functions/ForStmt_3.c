void a(){
	int b = 0;
	int i = 0; {{C}}
	for({{A: int i = 0;}} {{B: b}} {{C:  }}; i < 100; i++){}
}

/*
 * check-name: init-stmt in for
 * A != B; B != C; C != A
 */
