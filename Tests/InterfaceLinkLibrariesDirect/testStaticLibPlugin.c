extern int testStaticLibWithPlugin1(void);
extern int testStaticLibPluginExtra(void);
int testStaticLibPlugin(void)
{
  return testStaticLibWithPlugin1() + testStaticLibPluginExtra();
}
