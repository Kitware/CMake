enable_language(C)

set(prototypes [[
#include <stdio.h>
#include <zlib.h>
#include <resolv.h>
int func1();
int func2();
int func3();
int func4();
int func5();
int func6();
int func7();
]])
set(prototypes_objc [[
#import <CoreFoundation/CoreFoundation.h>
]])
set(impl [[
{
  printf("%p %p\n", compress, res_close);
  return func1() + func2() + func3() + func4() + func5() + func6() + func7();
}
]])
set(impl_objc [[
{
  CFStringRef cfStr = CFSTR("This is a string");
  printf("%p %p %ld\n", compress, res_close, (long)CFStringGetLength(cfStr));
  return func1() + func2() + func3() + func4() + func5() + func6() + func7();
}
]])

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/mainOuter.m
  "${prototypes}\n${prototypes_objc}\nint main(int argc, char** argv) ${impl_objc}"
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/funcOuter.c
  "${prototypes}\nint funcOuter() ${impl}"
)

foreach(i RANGE 1 5)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/func${i}.c
    "int func${i}() { return 32 + ${i}; }\n"
  )
endforeach()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ExternalFrameworks/CMakeLists.txt
[[
cmake_minimum_required(VERSION 3.18)
project(ExternalFrameworks)
add_library(staticFrameworkExt STATIC func6.c)
add_library(sharedFrameworkExt SHARED func7.c)
set_target_properties(staticFrameworkExt PROPERTIES FRAMEWORK TRUE)
set_target_properties(sharedFrameworkExt PROPERTIES FRAMEWORK TRUE)
]]
)

foreach(i RANGE 6 7)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ExternalFrameworks/func${i}.c
    "int func${i}() { return 32 + ${i}; }\n"
  )
endforeach()

add_custom_target(prebuildDependencies ALL
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_CURRENT_BINARY_DIR}/ExternalFrameworks -B ${CMAKE_CURRENT_BINARY_DIR}/ExternalFrameworks/build -G Xcode
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/ExternalFrameworks/build --target staticFrameworkExt sharedFrameworkExt --config Debug
)
add_executable(app1 mainOuter.m)
add_library(static1 STATIC funcOuter.c)
add_library(shared1 SHARED funcOuter.c)
add_library(module1 MODULE funcOuter.c)
add_library(obj1    OBJECT funcOuter.c)
add_library(staticFramework1 STATIC funcOuter.c)
add_library(sharedFramework1 SHARED funcOuter.c)
set_target_properties(staticFramework1 PROPERTIES FRAMEWORK TRUE)
set_target_properties(sharedFramework1 PROPERTIES FRAMEWORK TRUE)
add_dependencies(app1 prebuildDependencies)
add_dependencies(static1 prebuildDependencies)
add_dependencies(shared1 prebuildDependencies)
add_dependencies(module1 prebuildDependencies)
add_dependencies(obj1 prebuildDependencies)
add_dependencies(staticFramework1 prebuildDependencies)
add_dependencies(sharedFramework1 prebuildDependencies)

add_library(static2 STATIC func1.c)
add_library(shared2 SHARED func2.c)
add_library(obj2    OBJECT func3.c)
add_library(staticFramework2 STATIC func4.c)
add_library(sharedFramework2 SHARED func5.c)
set_target_properties(staticFramework2 PROPERTIES FRAMEWORK TRUE)
set_target_properties(sharedFramework2 PROPERTIES FRAMEWORK TRUE)

# Pick some external libraries that are always present in the Xcode SDK
find_library(libz z REQUIRED)
find_library(libresolv resolv REQUIRED)
find_library(CoreFoundation CoreFoundation REQUIRED)
add_library(imported2 UNKNOWN IMPORTED)
set_target_properties(imported2 PROPERTIES IMPORTED_LOCATION ${libz})

# Save these for the check script to use
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/foundLibs.cmake "
set(libz \"${libz}\")
set(libresolv \"${libresolv}\")
set(CoreFoundation \"${CoreFoundation}\")
")

set(mainTargets
    app1
    static1
    shared1
    module1
    obj1
    staticFramework1
    sharedFramework1
)
set(linkToThings
    static2
    shared2
    obj2
    staticFramework2
    sharedFramework2
    imported2
    ${libresolv}
    ${CoreFoundation}
    "${CMAKE_CURRENT_BINARY_DIR}/ExternalFrameworks/build/Debug/sharedFrameworkExt.framework"
    "${CMAKE_CURRENT_BINARY_DIR}/ExternalFrameworks/build/Debug/staticFrameworkExt.framework"
)

foreach(mainTarget IN LISTS mainTargets)
  foreach(linkTo IN LISTS linkToThings)
    target_link_libraries(${mainTarget} PRIVATE ${linkTo})
  endforeach()
endforeach()
