
#if __cplusplus >= 201103L
#  error "invalid standard value"
#endif
int func(int A, int B)
{
  // Verify that we have at least c++14
  if (A < B) {
    return A + B;
  } else {
    return B * A;
  }
}
