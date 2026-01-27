#include <cmath>
#include <format>

#include <MathLogger.h>

// TODO3: If the TUTORIAL_USE_SSE2 definition is set, include
//        the <emmintrin.h> header

namespace {

mathlogger::Logger Logger;

#if defined(TUTORIAL_USE_GNU_BUILTIN)
typedef double v2df __attribute__((vector_size(16)));

double gnu_mysqrt(double x)
{
  v2df root = __builtin_ia32_sqrtsd(v2df{ x, 0.0 });
  double result = root[0];
  Logger.Log(std::format("Computed sqrt of {} to be {} with GNU-builtins\n", x,
                         result));
  return result;
}
#elif defined(TUTORIAL_USE_SSE2)
double sse2_mysqrt(double x)
{
  __m128d root = _mm_sqrt_sd(_mm_setzero_pd(), _mm_set_sd(x));
  double result = _mm_cvtsd_f64(root);
  Logger.Log(
    std::format("Computed sqrt of {} to be {} with SSE2\n", x, result));
  return result;
}
#endif

// a hack square root calculation using simple operations
double fallback_mysqrt(double x)
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

    Logger.Log(std::format("Computing sqrt of {} to be {}\n", x, result));
  }
  return result;
}

double mysqrt(double x)
{
#if defined(TUTORIAL_USE_GNU_BUILTIN)
  return gnu_mysqrt(x);
#elif defined(TUTORIAL_USE_SSE2)
  return sse2_mysqrt(x);
#else
  return fallback_mysqrt(x);
#endif
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
