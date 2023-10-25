void OneFunc(void);

void TwoFunc(void)
{
  static int i = 0;
  ++i;
  if (i == 1) {
    OneFunc();
  }
}
