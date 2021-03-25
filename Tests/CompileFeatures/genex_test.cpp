#ifndef EXPECT_FINAL
#  error EXPECT_FINAL not defined
#endif
#ifndef EXPECT_INHERITING_CONSTRUCTORS
#  error EXPECT_INHERITING_CONSTRUCTORS not defined
#endif
#ifndef EXPECT_INHERITING_CONSTRUCTORS_AND_FINAL
#  error EXPECT_INHERITING_CONSTRUCTORS_AND_FINAL not defined
#endif
#ifndef EXPECT_OVERRIDE_CONTROL
#  error EXPECT_OVERRIDE_CONTROL not defined
#endif

#ifdef TEST_CXX_STD
#  if !HAVE_CXX_STD_11
#    error HAVE_CXX_STD_11 is false with CXX_STANDARD == 11
#  endif
#  if HAVE_CXX_STD_14 && !defined(ALLOW_LATER_STANDARDS)
#    error HAVE_CXX_STD_14 is true with CXX_STANDARD == 11
#  endif
#  if HAVE_CXX_STD_17 && !defined(ALLOW_LATER_STANDARDS)
#    error HAVE_CXX_STD_17 is true with CXX_STANDARD == 11
#  endif
#  if HAVE_CXX_STD_20 && !defined(ALLOW_LATER_STANDARDS)
#    error HAVE_CXX_STD_20 is true with CXX_STANDARD == 11
#  endif
#  if HAVE_CXX_STD_23 && !defined(ALLOW_LATER_STANDARDS)
#    error HAVE_CXX_STD_23 is true with CXX_STANDARD == 11
#  endif
#endif

#if !HAVE_OVERRIDE_CONTROL
#  if EXPECT_OVERRIDE_CONTROL
#    error "Expect override control feature"
#  endif
#else
#  if !EXPECT_OVERRIDE_CONTROL
#    error "Expect no override control feature"
#  endif
#endif

#if !HAVE_AUTO_TYPE
#  error Expect cxx_auto_type support
#endif

#if !HAVE_INHERITING_CONSTRUCTORS
#  if EXPECT_INHERITING_CONSTRUCTORS
#    error Expect cxx_inheriting_constructors support
#  endif
#else
#  if !EXPECT_INHERITING_CONSTRUCTORS
#    error Expect no cxx_inheriting_constructors support
#  endif
#endif

#if !HAVE_FINAL
#  if EXPECT_FINAL
#    error Expect cxx_final support
#  endif
#else
#  if !EXPECT_FINAL
#    error Expect no cxx_final support
#  endif
#endif

#if !HAVE_INHERITING_CONSTRUCTORS_AND_FINAL
#  if EXPECT_INHERITING_CONSTRUCTORS_AND_FINAL
#    error Expect cxx_inheriting_constructors and cxx_final support
#  endif
#else
#  if !EXPECT_INHERITING_CONSTRUCTORS_AND_FINAL
#    error Expect no combined cxx_inheriting_constructors and cxx_final support
#  endif
#endif

int main()
{
}
