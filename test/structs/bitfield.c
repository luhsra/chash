struct B {
        {{A}} unsigned int field:7;
        {{B}} unsigned int field:8;
};

struct B a;

/*
 * check-name: Bitfield lengths are checked
 * assert-ast: A != B
 * assert-obj: A == B
 * obj-not-diff: true
 *
 */
