struct foobar { {{A}}
union foobar {  {{B}}
    int first;
    char second;
    unsigned long third;
};

struct foobar foo; {{A}}
union foobar foo;  {{B}}

/*
 * check-name: struct vs union
 * assert-obj: A != B
 */

