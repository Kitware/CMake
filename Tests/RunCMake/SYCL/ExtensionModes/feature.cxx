#ifndef CXX_LANGUAGE_FLAG
#  error "Missing CXX language flags"
#endif
#ifdef SYCL_LANGUAGE_FLAG
#  error "CXX compilation inherited SYCL flags"
#endif
int feature_cxx()
{
  return 0;
}
