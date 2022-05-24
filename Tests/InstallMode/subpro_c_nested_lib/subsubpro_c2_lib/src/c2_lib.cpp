#include <iostream>

#include <c1_lib.h>
#include <c2_lib.h>

using namespace std;

void c2_hello()
{
  cout << "Hello from c2_lib and also..." << endl;
  c1_hello();
}
