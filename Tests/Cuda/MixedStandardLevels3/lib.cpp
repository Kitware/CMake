
int func(int A, int B)
{
  // Verify that we have at least c++14
  auto mult_func = [](auto a, auto b) { return a * b; };
  return mult_func(A, B);
}
