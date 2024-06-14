void TwoFunc(void);

void OneFunc(void)
{
  static int i = 0;
  ++i;
  if (i == 1) {
    TwoFunc();
  }
}
