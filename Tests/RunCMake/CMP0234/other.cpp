#if defined(EXPECT_STRICT)
#  if !defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL != 0
#    error "Conformant preprocessor is not used."
#  endif
#elif defined(EXPECT_EXTENSIONS)
#  if !defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL != 1
#    error "Traditional preprocessor is not used."
#  endif
#else
#  error "Neither EXPECT_STRICT nor EXPECT_EXTENSIONS is defined."
#endif

extern "C" int other(void)
{
  return 0;
}
