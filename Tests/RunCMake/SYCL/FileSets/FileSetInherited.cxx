#if !defined(LANG_SYCL) || defined(LANG_CXX)
#  error "FileSetInherited.cxx did not compile as SYCL"
#endif

int fileset_inherited()
{
  return 0;
}
