struct foobar{
	int gelb;
	char gruen;
	unsigned long magenta; {{B}}
};

int dasGehtDochNicht; {{A}}

/*
 * check-name: struct 2
 * B != A
 */

