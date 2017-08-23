
#include "skipSource/qItemA.hpp"
#include "skipSource/qItemB.hpp"
#include "skipSource/qItemC.hpp"
#include "skipSource/qItemD.hpp"

int main(int, char**)
{
  QItemA itemA;
  QItemB itemB;
  QItemC itemC;
  QItemD itemD;

  // Fails to link if the symbol is not present.
  return 0;
}
