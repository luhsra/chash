extern const int i;

extern int foo();

int changed() {
  return i;
}

int unchanged() {
  return 0;
}

int main() {
  int x = foo();
  x += unchanged();
  return x;
}

