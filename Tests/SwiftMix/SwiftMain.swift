import Foundation

@objc class SwiftMainClass : NSObject {
  class func SwiftMain(argc:Int, argv:UnsafePointer<UnsafePointer<CChar>>) -> Int32 {
    dump("argc: \(argc)")
#if swift(>=3.0)
    for i in 0 ..< argc {
      let argi = String(cString:argv[i]);
      dump("arg[\(i)]: \(argi)");
    }
#else
    for (var i = 0; i < argc; ++i) {
      let argi = String.fromCString(argv[i])
      dump("arg[\(i)]: \(argi)");
    }
#endif
    return 0;
  }
}
