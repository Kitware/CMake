#include <sycl/sycl.hpp>

template <typename Kernel>
int run_kernel()
{
  sycl::queue queue;
  int* value = sycl::malloc_shared<int>(1, queue);
  if (!value) {
    return 1;
  }
  *value = 0;
  queue.single_task<Kernel>([=] { *value = 42; });
  queue.wait_and_throw();
  int result = *value;
  sycl::free(value, queue);
  return result == 42 ? 0 : 1;
}
