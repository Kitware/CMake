#if defined(EXPECT_CXX)
#  if !defined(LANG_CXX) || defined(LANG_SYCL)
#    error "FileSetSource.cxx did not compile as CXX"
#  endif
#elif defined(EXPECT_SYCL)
#  if !defined(LANG_SYCL) || defined(LANG_CXX)
#    error "FileSetSource.cxx did not compile as SYCL"
#  endif
#else
#  error "Missing expected language"
#endif

int fileset_source()
{
  return 0;
}
