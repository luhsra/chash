static const int i = 1;

static const int j = i * 10;

int k = i + j + 42;

/*
 * check-name: init from static const global variables
 * references: j:export_def_use/variables/static_const_global.c -> i:export_def_use/variables/static_const_global.c, k -> i:export_def_use/variables/static_const_global.c j:export_def_use/variables/static_const_global.c
 */
