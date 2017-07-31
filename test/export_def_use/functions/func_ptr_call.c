int func() { return 0; }

int (*fp)() = &func;

void foo() { int i = fp(); }

/*
 * check-name: call of function pointer
 * references: fp:export_def_use/functions/func_ptr_call.c -> func:export_def_use/functions/func_ptr_call.c, foo:export_def_use/functions/func_ptr_call.c -> fp:export_def_use/functions/func_ptr_call.c
 */
