struct test{
	int a;
	int b;
}

struct test t[4] = {[1].b = 5, [3].a = -9; [0].b = 42}; {{A}}
struct test t[4] = {[2].b = 5, [3].a = -9; [0].b = 42}; {{B}} 

/*
 * check-name: DesignatedInitExpr
 * B != A
 */
