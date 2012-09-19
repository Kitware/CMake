
#ifndef CMAKE_IS_FUN
#error Expect CMAKE_IS_FUN definition
#endif

#if CMAKE_IS != Fun
#error Expect CMAKE_IS=Fun definition
#endif


template<bool test>
struct CMakeStaticAssert;

template<>
struct CMakeStaticAssert<true> {};

static const char fun_string[] = CMAKE_IS_;
#ifndef NO_SPACES_IN_DEFINE_VALUES
static const char very_fun_string[] = CMAKE_IS_REALLY;
#endif

enum {
  StringLiteralTest1 = sizeof(CMakeStaticAssert<sizeof(CMAKE_IS_) == sizeof("Fun")>)
#ifndef NO_SPACES_IN_DEFINE_VALUES
  ,
  StringLiteralTest2 = sizeof(CMakeStaticAssert<sizeof(CMAKE_IS_REALLY) == sizeof("Very Fun")>)
#endif
};

int main(int argc, char **argv)
{
  return 0;
}
