enum {
    REL_NONE = 0,
    REL_SYMBOLIC = -100,
    REL_SYM_OR_REL,
};

unsigned long a;

int foo() {
    if (a == REL_SYM_OR_REL) a++;
    if (a == REL_SYM_OR_REL) a++; {{A}}
    {{B}}
}

/*
 * check-name: Directly use enum values (also doubled)
 * obj-not-diff: yes
 * assert-ast: A != B
 */
