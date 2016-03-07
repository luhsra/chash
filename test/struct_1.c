struct foobar{
	int gelb;
	char gruen;
	unsigned long magenta; {{B}}
};

int dasGehtDochNicht = 0; {{A}}

/*
 * check-name: Definition of Global Variable
 * B != A
 */

