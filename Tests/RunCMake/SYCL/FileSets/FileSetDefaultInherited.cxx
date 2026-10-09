#if !defined(LANG_SYCL) || defined(LANG_CXX)
#  error "FileSetDefaultInherited.cxx did not compile as SYCL"
#endif

int fileset_default_inherited()
{
  return 0;
}
