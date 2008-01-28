extern int generated_by_testExe1();
extern int generated_by_testExe3();
extern int testLib2();
extern int testLib3();
extern int testLib4();

int main()
{
  return (testLib2() + generated_by_testExe1() + testLib3() + testLib4()
          + generated_by_testExe3());
}
