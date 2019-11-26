#ifndef __OBJC__
#  error "Compiler is not an Objective-C compiler."
#endif

#import <Foundation/Foundation.h>
#include <iostream>

int main()
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  std::cout << "Hello World" << std::endl;
  [pool release];
  return 0;
}
