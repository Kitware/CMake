#if defined(EXPECT_STRICT)
/* /Zc:__STDC__ defines __STDC__ as 1 and disables the MSVC and UCRT
   extensions.  */
#  if !defined(__STDC__) || __STDC__ != 1
#    error "__STDC__ is not defined to 1."
#  endif
#elif defined(EXPECT_EXTENSIONS)
/* Without /Zc:__STDC__ the compiler does not define __STDC__.  */
#  if defined(__STDC__)
#    error "__STDC__ is defined unexpectedly."
#  endif
#else
#  error "Neither EXPECT_STRICT nor EXPECT_EXTENSIONS is defined."
#endif

int main(void)
{
  return 0;
}
