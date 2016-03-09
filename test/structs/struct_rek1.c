struct barfoo;

struct foobar{
	int a;
	struct barfoo *b;
	long c;
};
{{A}}
struct barfoo{
	long a;
	struct foobar *b;
	int c;	{{B}}
};

struct foobar foo;
/*
 * check-name: struct recursive 1
 * obj-not-diff: Ja
 * B != A
 */

