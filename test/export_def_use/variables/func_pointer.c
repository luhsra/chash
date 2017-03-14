int func() { return 1; }

int (*fp)() = &func;

/*
 * check-name: function pointer
 * references: fp -> func
 */
