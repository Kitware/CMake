bool CheckSqrt(double val, double sqrt)
{
  double pow2 = sqrt * sqrt;
  double delta = val > pow2 ? val - pow2 : pow2 - val;

  // Close enough!
  return delta < 0.1;
}
