struct peter{
	int a;
	char b;
	char *c;
};
struct paul{
	int q;
	struct peter lankton;
};

struct paul lankton = {42, {5, 'a', "Wurstl"}}; {{A}}
struct paul lankton = {42, {5, 'b', "yo"}}; {{B}}
/*
 * check-name: Initlist struct MD
 * B != A
 */
