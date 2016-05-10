struct s { {{A}}
  int a;   {{A}}
  int b;   {{A}}
};         {{A}}

enum e {ea, eb}; {{A}}

typedef struct s s_type; {{A}}

{{B}}

/*
 * check-name: unused Decls
 * assert-ast: A == B
 */
