int func() { return 0; }

void foo() { int (*fp)() = &func; }

/*
 * check-name: local function ptr
 * references: foo:export_def_use/functions/function_ptr.c -> func:export_def_use/functions/function_ptr.c
 */
