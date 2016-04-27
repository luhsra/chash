struct peter{
	int a;
	char b;
	char *c;
};
struct paul{
	int q;
	struct peter lankton;
};

struct paul i; {{A}}
struct paul j; {{B}}
/*
 * check-name: struct MD
 * assert-obj: B != A
 */
//TODO: sollten die nicht gleich sein? oder ist name ausreichend fuer diff?
