

constexpr int func(int A, int B)
{
#if defined(_MSC_VER) && _MSC_VER < 1913
  // no suppport for extended constexpr
  return B * A;
#else
  // Verify that we have at least c++14
  if (A < B) {
    return A + B;
  } else {
    return B * A;
  }
#endif
}
