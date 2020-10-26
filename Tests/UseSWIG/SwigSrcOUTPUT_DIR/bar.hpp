#pragma once

namespace Bar {

class Math
{
public:
  void add(int value);
  int get_sum() const;

private:
  int sum_ = 0;
};

} // namespace cs
