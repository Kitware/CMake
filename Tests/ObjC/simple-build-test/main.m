#import <Foundation/Foundation.h>
#import "foo.h"

int main(int argc, char **argv)
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  Foo *theFoo = [[Foo alloc] init];
  theFoo.age = [NSNumber numberWithInt:argc];
  NSLog(@"%d\n",[theFoo.age intValue]);
  [pool release];
  return 0;
}
