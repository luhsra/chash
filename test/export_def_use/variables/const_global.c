const int i = 1;

const int j = i * 10;

int k = i + j + 42;

/*
 * check-name: init from const globals
 * references: j:export_def_use/variables/const_global.c -> i:export_def_use/variables/const_global.c, k:export_def_use/variables/const_global.c -> i:export_def_use/variables/const_global.c j:export_def_use/variables/const_global.c
 */
