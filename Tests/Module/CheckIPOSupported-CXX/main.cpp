int foo();
int bar();

int main()
{
  if (foo() != bar()) {
    return 1;
  }
  return 0;
}
