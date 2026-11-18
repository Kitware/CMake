int NoMocNeeded();
int NoMocNeeded()
{
  return 0;
}

#ifdef WITH_MAIN
int main()
{
  return NoMocNeeded();
}
#endif
