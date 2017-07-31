int func() { return 1; }

int (*fp)() = &func;

/*
 * check-name: function pointer
 * references: fp:export_def_use/variables/func_pointer.c -> func:export_def_use/variables/func_pointer.c
 */
