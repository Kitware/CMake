#ifdef __cplusplus
#  error "FileSetAsC.cxx was parsed as C++"
#endif

#if !defined(LANG_C) || defined(LANG_CXX)
#  error "FileSetAsC.cxx did not compile as C"
#endif

int fileset_as_c(void)
{
  return 0;
}
