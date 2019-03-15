extern int func(void);
extern int lib1_func(void);
int main(void)
{
  return func() + lib1_func();
}
