
#pragma once
#include "file1.h"

struct result_type_dynamic
{
  int input;
  int sum;
  bool from_static;
};

result_type_dynamic __device__ file2_func(int x);
