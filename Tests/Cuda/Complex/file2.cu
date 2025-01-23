
#include "file2.h"

result_type __device__ file1_func(int x);

result_type_dynamic __device__ file2_func(int x)
{
  if (x != 42) {
    result_type const r = file1_func(x);
    result_type_dynamic const rd{ r.input, r.sum, true };
    return rd;
  } else {
    result_type_dynamic const rd{ x, x * x * x, false };
    return rd;
  }
}
