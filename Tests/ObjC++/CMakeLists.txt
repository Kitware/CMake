# a simple objc++ test case that uses Cocoa framework
PROJECT (objc++)

ADD_EXECUTABLE (objc++ objc++.mm)
TARGET_LINK_LIBRARIES(objc++ "-framework Cocoa")

