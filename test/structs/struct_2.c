struct foobar{
	int gelb;
	char gruen;
	unsigned long magenta; {{B}}
};
{{A}}
int dasGehtDochNicht;

/*
 * check-name: struct 2
 * B == A
 */

