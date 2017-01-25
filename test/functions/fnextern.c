extern void lua_strtonum() {} {{A}}

extern void lua_stringtonum() {} {{B}}

extern void lua_strtonum() {} {{C}}
void unused_prototype(); {{C}}

extern void lua_strtonum() {} {{D}}
extern void unused_prototype(); {{D}}

extern void lua_strtonum() {} {{E}}
void unused_prototype(); {{E}}
void unused_prototype() {} {{E}}

extern void lua_strtonum() {} {{F}}
void unused_prototype() {} {{F}}

extern void lua_strtonum() {} {{G}}
extern void unused_prototype() {} {{G}}

/*
 * check-name: function name and extern
 * assert-obj: A != B
 * assert-ast: A == C, C == D, D != E, E == F
 * obj-not-diff: maybe
 */
