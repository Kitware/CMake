#include <omp.h>
int main(void)
{
#ifndef _OPENMP
  breaks_on_purpose
#endif
}
