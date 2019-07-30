import Foundation

@objc class SwiftMainClass : NSObject {
  @objc class func SwiftMain() -> Int32 {
    dump("Hello World!");
    return 0;
  }
}
