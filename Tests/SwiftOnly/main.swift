dump("SwiftOnly")

#if SWIFTONLY
dump("SWIFTONLY defined")
#else
fatalError("SWIFTONLY NOT defined")
#endif
