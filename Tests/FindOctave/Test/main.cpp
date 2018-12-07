#include <iostream>
#include <oct.h>

// http://www.dm.unibo.it/~achilles/calc/octave.html/Standalone-Programs.html
int main(void)
{
  int n = 2;
  Matrix a_matrix = Matrix(n, n);
  for (octave_idx_type i = 0; i < n; i++) {
    for (octave_idx_type j = 0; j < n; j++) {
      a_matrix(i, j) = (i + 1) * 10 + (j + 1);
    }
  }

  std::cout << a_matrix << std::endl;

  return EXIT_SUCCESS;
}
