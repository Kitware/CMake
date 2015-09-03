import Foundation

@objc class SwiftMainClass : NSObject {
  class func SwiftMain(argc:Int, argv:UnsafePointer<UnsafePointer<CChar>>) -> Int32 {
    dump("argc: \(argc)")
    for (var i = 0; i < argc; ++i) {
      let argi = String.fromCString(argv[i])
      dump("arg[\(i)]: \(argi)");
    }
    return 0;
  }
}
