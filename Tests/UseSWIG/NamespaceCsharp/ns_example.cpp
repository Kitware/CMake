#include "ns_example.hpp"

namespace ns {

void my_class_in_namespace::add(int value)
{
  Sum += value;
}

int my_class_in_namespace::get_sum() const
{
  return Sum;
}
}
