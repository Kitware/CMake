#if defined(EXPECT_PERMISSIVE)
// Compiles only in permissive mode: /permissive- rejects binding
// a temporary to a non-const lvalue reference.
struct S
{
};

S& temporary = S();

int main()
{
  return 0;
}
#elif defined(EXPECT_NON_PERMISSIVE)
// Compiles only in non-permissive mode: /permissive- makes the alternative
// tokens keywords, while the default mode needs <iso646.h> for them.
int main()
{
  return 1 and 0;
}
#else
#  error "Neither EXPECT_PERMISSIVE nor EXPECT_NON_PERMISSIVE is defined."
#endif
