struct foobar{
	int gelb;
	char gruen;
	unsigned long magenta; {{B}}
};

int dasGehtDochNicht; {{A}}

/*
 * check-name: Definition of Global Variable
 * B != A
 */

