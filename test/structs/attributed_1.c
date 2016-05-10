struct IOAPICID {
  unsigned int reserved_2:24,
               ID:4, ///< IOAPIC Identification, R/W
               reserved_1:4;
} {{A: ;}} {{B: __attribute__((packed));}}

struct IOAPICID a;

/*
 * check-name: attribute packed
 * assert-ast: A != B
 * 
 * WARNING: the object can indeed become different because of alignment issues,
 * so this does not hold: obj: A == B
 */
