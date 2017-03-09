int func() { return 0; }

void foo() { int (*fp)() = &func; }

/*
 * check-name: local function ptr
 * references: foo -> func
 * no-references: func
 * no_entry: fp
 */
