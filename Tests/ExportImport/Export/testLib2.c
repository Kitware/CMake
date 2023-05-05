
#ifndef FROM_compileOnly
#  error "Usage requirements from `compileOnly` not found"
#endif

extern int testLib1(void);

int testLib2(void)
{
  return testLib1();
}
