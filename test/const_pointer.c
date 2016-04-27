void cdcmd(void)
{
        const char *dest; {{A}}
        const char * const dest; {{B}}

        char c;
        c = dest[2];
}
/*
 * check-name: qualifiers in subtype
 * obj-not-diff: maybe
 * assert-ast: A != B
 * assert-obj: A == B
 */
