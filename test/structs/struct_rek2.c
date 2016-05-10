struct barfoo;

struct foobar {
	int a;
	struct barfoo *b;
	long c; {{B}}
};
{{A}}
struct barfoo {
	long a;
	struct foobar *b;
	int c;
};

struct barfoo foo;

/*
 * check-name: struct recursive 2; struct with different members but not instantiated
 * obj-not-diff: Ja
 * assert-ast: A == B
 */
//TODO: remove obj-?; actually exactly the same as struct_rek1.c...
