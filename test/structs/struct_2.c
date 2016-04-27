struct foobar{
	int gelb;
	char gruen;
	unsigned long magenta; {{B}}
};
{{A}}
int dasGehtDochNicht;

/*
 * check-name: struct 2
 * assert-obj: B == A
 */
//TODO: muss doch != sein?! oder nicht weil nicht verwendet wird?
