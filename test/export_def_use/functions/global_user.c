int global = 1;

int arr[2];

int global_user() {
  int local = 2;

  int i = other_foo();

  return arr[global] = local + global + i;
}

/*
 * check-name: function using global variables
 * references: global_user:export_def_use/functions/global_user.c -> global:export_def_use/functions/global_user.c arr:export_def_use/functions/global_user.c other_foo:export_def_use/functions/global_user.c
 */
