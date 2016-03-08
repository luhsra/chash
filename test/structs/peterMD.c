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
 * B != A
 */
