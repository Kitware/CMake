#include <cmath>
#include <format>

// TODO5: Replace <iostream> with <MathLogger.h>

#include <iostream>

namespace {

// TODO6: Instantiate a logger inside the anonymous namespace

// a hack square root calculation using simple operations
double mysqrt(double x)
{
  if (x <= 0) {
    return 0;
  }

  double result = x;

  // do ten iterations
  for (int i = 0; i < 10; ++i) {
    if (result <= 0) {
      result = 0.1;
    }
    double delta = x - (result * result);
    result = result + 0.5 * delta / result;

    // TODO7: Use the logger to log the message
    std::cout << std::format("Computing sqrt of {} to be {}\n", x, result);
  }
  return result;
}
}

namespace mathfunctions {
double sqrt(double x)
{
#ifdef TUTORIAL_USE_STD_SQRT
  return std::sqrt(x);
#else
  return mysqrt(x);
#endif
}
}
