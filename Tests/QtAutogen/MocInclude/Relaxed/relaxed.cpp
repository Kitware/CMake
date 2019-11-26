// AUTOMOC relaxed mode objects
#include "RObjA.hpp"
#include "RObjB.hpp"
#include "RObjC.hpp"

// Forward declaration
bool commonRelaxed();

int main(int argv, char** args)
{
  // Common tests
  if (!commonRelaxed()) {
    return -1;
  }

  // Relaxed tests
  RObjA rObjA;
  RObjB rObjB;
  RObjC rObjC;
  return 0;
}
