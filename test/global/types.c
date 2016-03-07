int a1; {{A}}
char a2; {{B}}
short a3; {{C}}

long a2; {{A}}
long long a3; {{B}}
float a1; {{C}}

double a3; {{A}}
long double a1; {{B}}
unsigned short a2; {{C}}
/*
 * check-name: More Types
 * B != A, C != B, A != C
 */

