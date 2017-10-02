int srcgenex(void)
{
  return 0;
}

int main(int argc, char* argv[])
{
#ifndef NAME
#error NAME not defined
#endif
  return NAME();
}
