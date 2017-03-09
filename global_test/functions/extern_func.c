extern void ext();

void func() { ext(); }

/*
 * check-name: use of extern function
 * references: func -> ext
 * no-references: ext
 * no-entry: ext
 */
