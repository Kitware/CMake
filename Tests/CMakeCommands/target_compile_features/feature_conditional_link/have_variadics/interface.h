template<int I, int... Is>
struct Interface;

template<int I>
struct Interface<I>
{
  static int accumulate()
  {
    return I;
  }
};

template<int I, int... Is>
struct Interface
{
  static int accumulate()
  {
    return I + Interface<Is...>::accumulate();
  }
};
