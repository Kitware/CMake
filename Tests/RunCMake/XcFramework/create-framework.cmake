set(CMAKE_FRAMEWORK ON)
include(create-library-common.cmake)
install(FILES mylib/include/mylib/mylib.h DESTINATION lib/mylib.framework/Headers)
