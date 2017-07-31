int func_no_def();

void func() { func_no_def(); }

/*
 * check-name: use of non-defined function
 * references: func:export_def_use/functions/non_defined_function.c -> func_no_def:export_def_use/functions/non_defined_function.c
 */
