#define _STR(x) #x
#define STR(x) _STR(x)

enum vocalization
{
  meow,  // tame cat
  roar,  // wild cat
  whine, // tame dog
  growl  // wild dog
};

#if __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
static_assert(NOISE == EXPECTED,
              "expected " STR(EXPECTED) ", got " STR(NOISE));
#else
typedef int test[(NOISE == EXPECTED) ? 1 : -1];
#endif

int main()
{
  return 0;
}
