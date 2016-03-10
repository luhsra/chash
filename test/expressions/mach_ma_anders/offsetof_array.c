int array[10];

size_t a = offsetof(array, array[2]); {{A}}
size_t a = 8; {{B}}

/*
 * check-name: offsetof array
 * obj-not-diff: might
 * B != A
 */
