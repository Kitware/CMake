void ThreeFunction();

void TwoFunction()
{
  static int count = 0;
  if( count == 0 ) {
    ++count;
    ThreeFunction();
  }
}
