#ifndef __OBJC__
#  error "Compiler is not an Objective-C compiler."
#endif

#import <Foundation/Foundation.h>

int main()
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  [pool release];
  return 0;
}
