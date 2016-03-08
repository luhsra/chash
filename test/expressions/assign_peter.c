struct peter{
	int a;
	char b;
	char *c;
};

struct peter lankton = {5, 'a', "Wurstl"}; {{A}}
struct peter lankton = {5, 'b', "yo"}; {{B}}
/*
 * check-name: Initlist struct
 * B != A
 */
