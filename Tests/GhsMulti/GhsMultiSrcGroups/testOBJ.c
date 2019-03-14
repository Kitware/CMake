extern int func2a(void);
extern int func3a(void);
extern int func4a(void);
extern int func5a(void);
extern int func6a(void);
extern int func7a(void);

int funcOBJ(void)
{
  return func2a() + func3a() + func4a() + func5a() + func6a() + func7a();
}
