#if !defined(LANG_SYCL) || defined(LANG_CXX)
#  error "FileSetHead.cxx did not compile as SYCL"
#endif

int fileset_head()
{
  return 0;
}
