void g();

void f() { g(); }

void g() { f(); }

/*
 * check-name: mutual recursion
 * references: f -> g, g -> f
 */
