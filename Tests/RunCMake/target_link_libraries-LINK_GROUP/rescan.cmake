
enable_language(C)

# Feature RESCAN
add_library(static1 STATIC func1.c func3.c)
add_library(static2 STATIC func2.c)

add_executable(main main.c)
target_link_libraries(main PRIVATE "$<LINK_GROUP:RESCAN,static1,static2>")
