/* Produce an error if the object compiled from this source is used.  */
extern int testStaticLibWithPlugin_linked_before_testStaticLibPlugin(void);
int testStaticLibPlugin(void)
{
  return testStaticLibWithPlugin_linked_before_testStaticLibPlugin();
}
