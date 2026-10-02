extern "C" int moddef_answer();

int main()
{
  return moddef_answer() == 42 ? 0 : 1;
}
