extern int ext;

void func() { int i = 3 * ext; }

/*
 * check-name: use of extern variable
 * references: func -> ext
 * no-references: ext
 * no-entry: ext i
 */
