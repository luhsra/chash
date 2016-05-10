void func(int a, ...) {} {{A}}
void func(int a) {}     {{B}}

/*
 * check-name: multiple_args
 * assert-obj: A != B
 */
