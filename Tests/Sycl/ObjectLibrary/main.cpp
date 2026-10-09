int sycl_value();
int cxx_value();
int conflicting_sycl_value();
int conflicting_cxx_value();

int main()
{
  return sycl_value() + cxx_value() + conflicting_sycl_value() +
        conflicting_cxx_value() ==
      10
    ? 0
    : 1;
}
