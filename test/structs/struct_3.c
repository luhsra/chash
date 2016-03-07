struct foobar{
	int *gelb;
	char *gruen;
	unsigned long magenta; {{B}}
};

struct foobar *mkay; {{A}}
int rot_rot_usw;

/*
 * check-name: struct 3
 * B != A
 */

