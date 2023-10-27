void OneFunction(void);
void FourFunction(void);

void ThreeFunction(void)
{
  static int count = 0;
  if (count == 0) {
    ++count;
    FourFunction();
  }
  OneFunction();
}
