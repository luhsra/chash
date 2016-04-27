typedef struct foobar{
	int okay;
} used;

used first; {{A}}
int first; {{B}}
/*
 * check-name: typedef struct
 * obj-not-diff: might be the same size
 * assert-ast: A != B
 * assert-obj: A == B
 */
