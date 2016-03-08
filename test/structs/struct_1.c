struct foobar{
	int gelb;
	char gruen;
	unsigned long magenta; {{B}}
};

//int dasGehtDochNicht = 0; {{A}}
struct foobar foo;

/*
 * check-name: struct 1
 * B != A
 */

