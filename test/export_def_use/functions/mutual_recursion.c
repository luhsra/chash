void g();

void f() { g(); }

void g() { f(); }

/*
 * check-name: mutual recursion
 * references: f:export_def_use/functions/mutual_recursion.c -> g:export_def_use/functions/mutual_recursion.c, g:export_def_use/functions/mutual_recursion.c -> f:export_def_use/functions/mutual_recursion.c
 */
