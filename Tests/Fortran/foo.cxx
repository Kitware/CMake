extern "C" int foo(void);
int main()
{
  delete new int;
  return foo();
}
