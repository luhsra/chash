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
 * assert-ast: A == B
 * assert-obj: A == B
 */
//TODO: isn't this the same as struct_rek2? was != before,
//      but seems to be == as the unused/not instantiated struct is ignored
