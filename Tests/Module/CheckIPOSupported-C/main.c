int foo();
int bar();

int main(void)
{
  if (foo() != bar()) {
    return 1;
  }
  return 0;
}
