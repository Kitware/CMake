extern int FooObject(void);
extern int FooStatic(void);

int main(void)
{
  return FooObject() + FooStatic();
}
