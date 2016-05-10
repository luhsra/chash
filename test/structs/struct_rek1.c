struct barfoo;

struct foobar {
	int a;
	struct barfoo *b;
	long c;
};
{{A}}
struct barfoo {
	long a;
	struct foobar *b;
	int c;	{{B}}
};

struct foobar foo;

/*
 * check-name: struct recursive 1; struct with different members but not instantiated
 * obj-not-diff: Ja
 * assert-ast: A == B
 */
//TODO: remove obj-?
