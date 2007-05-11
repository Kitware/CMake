# a simple objc++ test case that uses Cocoa framework
PROJECT (ObjC++)

ADD_EXECUTABLE (ObjC++ objc++.mm)
TARGET_LINK_LIBRARIES(ObjC++ "-framework Cocoa")

