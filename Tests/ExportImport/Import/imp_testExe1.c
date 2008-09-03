extern int generated_by_testExe1();
extern int generated_by_testExe3();
extern int testLib2();
extern int testLib3();
extern int testLib4();
extern int testLib4lib();

/* Switch a symbol between debug and optimized builds to make sure the
   proper library is found from the testLib4 link interface.  */
#ifdef EXE_DBG
# define testLib4libcfg testLib4libdbg
#else
# define testLib4libcfg testLib4libopt
#endif
extern testLib4libcfg(void);

int main()
{
  return (testLib2() + generated_by_testExe1() + testLib3() + testLib4()
          + generated_by_testExe3() + testLib4lib() + testLib4libcfg());
}
