/* Produce an error if if the object compiled from this source is used.  */
extern int testStaticLibWithPlugin_linked_before_testStaticLibPluginExtra(
  void);
int testStaticLibPluginExtra(void)
{
  return testStaticLibWithPlugin_linked_before_testStaticLibPluginExtra();
}
