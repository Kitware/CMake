#if __has_include(<emmintrin.h>)
#  include <emmintrin.h>

double inspect(double x)
{
  __m128d root = _mm_sqrt_sd(_mm_setzero_pd(), _mm_set_sd(x));
  return _mm_cvtsd_f64(root);
}
#endif

int main()
{
  return 0;
}
