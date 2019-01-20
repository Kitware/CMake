#include "MathFunctions.h"
#include <iostream>

// include the generated table
#include "Table.h"

#include <cmath>

namespace mathfunctions {
namespace detail {
// a hack square root calculation using simple operations
double mysqrt(double x)
{
  if (x <= 0) {
    return 0;
  }

  // if we have both log and exp then use them
#if defined(HAVE_LOG) && defined(HAVE_EXP)
  double result = exp(log(x) * 0.5);
  std::cout << "Computing sqrt of " << x << " to be " << result << " using log"
            << std::endl;
#else
  // use the table to help find an initial value
  double result = x;
  if (x >= 1 && x < 10) {
    result = sqrtTable[static_cast<int>(x)];
  }

  // if we have both log and exp then use them

  // do ten iterations
  for (int i = 0; i < 10; ++i) {
    if (result <= 0) {
      result = 0.1;
    }
    double delta = x - (result * result);
    result = result + 0.5 * delta / result;
    std::cout << "Computing sqrt of " << x << " to be " << result << std::endl;
  }
#endif
  return result;
}
}
}
