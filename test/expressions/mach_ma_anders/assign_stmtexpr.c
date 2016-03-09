int a = {int x = 7; ++x;}; {{A}}
int a = {int x = 7}; {{B}}

/*
 * check-name: STMTEXPR
 * B != A
 */
