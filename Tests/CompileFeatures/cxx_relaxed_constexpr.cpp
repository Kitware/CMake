
struct X
{
  constexpr X()
    : n(5)
  {
    n *= 2;
  }
  int n;
};

constexpr int g(const int (&is)[4])
{
  X x;
  int r = x.n;
  for (int i = 0; i < 5; ++i)
    r += i;
  for (auto& i : is)
    r += i;
  return r;
}

int someFunc()
{
  constexpr int values[4] = { 4, 5, 6, 7 };
  constexpr int k3 = g(values);
  return k3 - 42;
}
