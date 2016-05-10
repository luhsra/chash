void a() {
    do {} while (1);         {{A}}
    do { break; } while (1); {{B}}
}

/*
 * check-name: break in do-while(1)
 * assert-obj: A != B
 */
