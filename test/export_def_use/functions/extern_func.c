extern void ext();

void func() { ext(); }

/*
 * check-name: use of extern function
 * references: func:export_def_use/functions/extern_func.c -> ext:export_def_use/functions/extern_func.c
 */
