#include <iostream>
#include "hello.h"
int Hello::Data = 0;
void Hello::real()
{
  return;
}
void hello()
{
  std::cout << "hello";
}
void Hello::operator delete[](void* ptr) {};
void Hello::operator delete(void* ptr) {};
