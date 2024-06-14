/* depends on NoDepF */
void NoDepF_func(void);

void NoDepE_func(void)
{
  static int firstcall = 1;
  if (firstcall) {
    firstcall = 0;
    NoDepF_func();
  }
}
