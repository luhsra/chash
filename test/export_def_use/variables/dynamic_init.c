int func() { return 1; }

int i;

void init() { i = func(); }

/*
 * check-name: uninitialized global; set in function
 * references: init:export_def_use/variables/dynamic_init.c -> i:export_def_use/variables/dynamic_init.c func:export_def_use/variables/dynamic_init.c
 */
