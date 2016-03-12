struct IOAPICID {
    unsigned int reserved_2;
    char ID;
    long reserved_1;
} {{A: ;}} {{B: __attribute__((packed));}}

struct IOAPICID a;


/*
 * check-name: attribute packed
 * A != B
 */
