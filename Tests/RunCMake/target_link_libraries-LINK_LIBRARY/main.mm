#import <Foundation/Foundation.h>
#import "foo.h"
#include <iostream>

int main(int argc, char **argv)
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  Foo *theFoo = [[Foo alloc] init];
  theFoo.age = [NSNumber numberWithInt:argc];
  NSLog(@"%d\n",[theFoo.age intValue]);
  std::cout << [theFoo.age intValue] << std::endl;
  [pool release];
  return 0;
}
