int func_no_def();

void func() { func_no_def(); }

/*
 * check-name: use of non-defined function
 * references: func-> func_no_def
 */
