void NoDepAFunction();
void OneFunction();
void TwoFunction();

void FourFunction()
{
  static int count = 0;
  if( count == 0 ) {
    ++count;
    TwoFunction();
  }
  OneFunction();
  NoDepAFunction();
}
