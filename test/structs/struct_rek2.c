struct barfoo;

struct foobar{
	int a;
	struct barfoo *b;
	long c; {{B}}
};
{{A}}
struct barfoo{
	long a;
	struct foobar *b;
	int c;
};

struct barfoo foo;
/*
 * check-name: struct recursive 2
 * obj-not-diff: Ja
 * assert-ast: A == B
 * assert-obj: A == B
 */
//TODO: war vorher !=; warum == ?
