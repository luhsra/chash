int a = 04711; {{A}}
int a = 4711; {{B}}
int a = 0x4711; {{C}}

/*
 * check-name: oktal,...
 * B != A, B != C, A != C
 */
