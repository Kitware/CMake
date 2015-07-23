@objc class SwiftMainClass {
  class func SwiftMain(argc:Int, argv:UnsafePointer<UnsafePointer<CChar>>) -> Int32 {
    println("argc: \(argc)")
    for (var i = 0; i < argc; ++i) {
      var argi = String.fromCString(argv[i])
      println("arg[\(i)]: \(argi)");
    }
    return 0;
  }
}
