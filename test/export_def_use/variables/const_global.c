const int i = 1;

const int j = i * 10;

int k = i + j + 42;

/*
 * check-name: init from const globals
 * references: j -> i, k -> i j
 */
