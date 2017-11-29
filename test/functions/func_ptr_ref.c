extern void foo();
extern void bar();


void f2() {
    void (*barfoo)() = &foo; {{A}}
    void (*barfoo)() = &bar; {{B}}
}

/*
 * check-name: Function pointer references
 * assert-obj: A != B
 */
