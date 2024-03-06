export module importable;

export int from_import()
{
  return 0;
}

extern "C++" {
int f()
{
  return from_import();
}
}
