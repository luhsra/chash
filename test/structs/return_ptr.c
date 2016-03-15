struct foobar{
	int l;
};

struct foobar *barfoo(){
	struct foobar foo = {.l = 1};
	return &foo;
}
{{A}}

/*
 * check-name: return recordptr
 */

