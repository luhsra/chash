int global = 1;

int arr[2];

int global_user() {
  int local = 2;

  int i = other_foo();

  return arr[global] = local + global + i;
}

/*
 * check-name: function using global variables
 * references: global_user -> global array
 * no-references: global arr
 * no-entry: local i
 */
