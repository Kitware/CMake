#if !defined(CXX_FLAGS) || defined(SYCL_FLAGS)
#  error "The isolated CXX flags were not used"
#endif
int sycl_value();
int main()
{
  return sycl_value() == 42 ? 0 : 1;
}
