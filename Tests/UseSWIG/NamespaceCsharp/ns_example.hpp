#pragma once

namespace ns {

class my_class_in_namespace
{
public:
  my_class_in_namespace()
    : Sum(0)
  {
  }

  void add(int value);
  int get_sum() const;

private:
  int Sum;
};
}
