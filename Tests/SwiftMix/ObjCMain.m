#import "SwiftMix-Swift.h"
int ObjCMain(int argc, char const* const argv[]) {
  if ([SwiftMainClass respondsToSelector:@selector(SwiftMain:argv:)]) {
    return [SwiftMainClass SwiftMain:argc argv:argv];
  }
  if ([SwiftMainClass respondsToSelector:@selector(SwiftMainWithArgc:argv:)]) {
    return [SwiftMainClass SwiftMainWithArgc:argc argv:argv];
  }
  return -1;
}
