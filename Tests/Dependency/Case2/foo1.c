extern int foo2(void);
int foo1(void) { return foo2(); }
int foo1_from_foo3(void) { return 0; }
