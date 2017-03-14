int func() { return 0; }

int (*fp)() = &func;

void foo() { int i = fp(); }

/*
 * check-name: call of function pointer
 * references: fp -> func, foo -> fp
 */
