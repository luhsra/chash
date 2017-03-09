int func() { return 1; }

int i;

void init() { i = func(); }

/*
 * check-name: uninitialized global; set in function
 * references: init -> i func
 */
