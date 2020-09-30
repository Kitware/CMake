#include <bar.hpp>
#include <foo.hpp>

namespace Foo {

void Math::add(int value)
{
  sum_ += value;
}

int Math::get_sum() const
{
  return sum_;
}
}

namespace Bar {

void Math::add(int value)
{
  sum_ += value;
}

int Math::get_sum() const
{
  return sum_;
}

} // namespace cs
