struct IOAPICID {
    unsigned int reserved_2:24,
             ID:4, ///< IOAPIC Identification, R/W
             reserved_1:4;
} {{A: ;}} {{B: __attribute__((packed));}}

struct IOAPICID a;


/*
 * check-name: attribute packed
 * obj-not-diff: object file isn't changed indeed!
 * A != B
 */
