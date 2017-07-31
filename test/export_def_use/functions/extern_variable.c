extern int ext;

void func() { int i = 3 * ext; }

/*
 * check-name: use of extern variable
 * references: func:export_def_use/functions/extern_variable.c -> ext:export_def_use/functions/extern_variable.c
 */
