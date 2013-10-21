
template<int I1, int I2 = 0, int I3 = 0, int I4 = 0>
struct Interface
{
  static int accumulate() { return I1 + I2 + I3 + I4; }
};
