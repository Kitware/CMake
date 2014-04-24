
// Dummy implementation. Test only the compiler feature.
namespace std {
  typedef decltype(sizeof(int)) size_t;
  template <class _E>
  class initializer_list
  {
    const _E* __begin_;
    size_t    __size_;

  };
}

template <typename T>
struct A
{
  A(std::initializer_list<T>) {}
};

void someFunc()
{
  A<int> as = { 1, 2, 3, 4 };
}
