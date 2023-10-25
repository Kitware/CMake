/* depends on NoDepE */
void NoDepE_func(void);

void NoDepF_func(void)
{
  static int firstcall = 1;
  if (firstcall) {
    firstcall = 0;
    NoDepE_func();
  }
}
