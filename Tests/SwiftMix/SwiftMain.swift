import Foundation

@objc class SwiftMainClass : NSObject {
  @objc class func SwiftMain() -> Int32 {
    dump("Hello World!");
#if FOO
    dump("FOO defined");
#else
    fatalError("FOO not defined");
#endif
#if BAR
    dump("BAR defined");
#else
    fatalError("BAR not defined");
#endif
#if CCOND
    fatalError("CCOND defined");
#else
    dump("CCOND not defined");
#endif
#if SWIFTCOND
    dump("SWIFTCOND defined");
#else
    fatalError("SWIFTCOND not defined");
#endif
    return 0;
  }
}
