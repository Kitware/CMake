
#include "file2.h"

result_type_dynamic __device__ file2_func(int x)
{
  const result_type r = file1_func(x);
  const result_type_dynamic rd{ r.input, r.sum, true };
  return rd;
}
