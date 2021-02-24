
cmake_minimum_required(VERSION 3.16...3.17)

enable_language(C)
enable_language(CXX)

add_library(shared_C SHARED func.c)
add_library(shared_CXX SHARED func.cxx)

add_library(static_cxx STATIC func.cxx)
target_compile_definitions(static_cxx PRIVATE BUILD_STATIC)

add_library(static1_C STATIC empty.c)
target_link_libraries (static1_C INTERFACE $<$<LINK_LANGUAGE:C>:shared_C>)

add_library(static2_C STATIC empty.c)
target_link_libraries (static2_C PRIVATE $<$<LINK_LANGUAGE:C>:shared_C>)


set (binary_dir "${CMAKE_BINARY_DIR}")
get_property (is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (is_multi_config)
  string (APPEND binary_dir "/Release")
endif()
add_library(import STATIC IMPORTED)
set_property(TARGET import PROPERTY IMPORTED_LOCATION "${binary_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}static1_C${CMAKE_STATIC_LIBRARY_SUFFIX}")
target_link_libraries (import INTERFACE $<$<LINK_LANGUAGE:C>:shared_C>)
target_link_libraries (import INTERFACE $<$<LINK_LANGUAGE:CXX>:shared_CXX>)


add_library(interface INTERFACE)
target_link_libraries (interface INTERFACE $<$<LINK_LANGUAGE:C>:shared_C>
                                           $<$<LINK_LANGUAGE:CXX>:shared_CXX>)


add_library(interface2 INTERFACE)
target_link_libraries (interface2 INTERFACE import)


add_library(static3 STATIC empty.c)
target_link_libraries (static3 PRIVATE interface)


add_library(LinkLibraries_lib SHARED lib.c)
target_link_libraries (LinkLibraries_lib PRIVATE $<$<LINK_LANGUAGE:C>:shared_C>)

add_library(LinkLibraries_lib2 SHARED lib.c)
target_link_libraries (LinkLibraries_lib2 PRIVATE $<$<LINK_LANGUAGE:C>:static1_C>)

add_library(LinkLibraries_lib3 SHARED lib.c)
target_link_libraries (LinkLibraries_lib3 PRIVATE $<$<LINK_LANGUAGE:C>:static2_C>)

add_executable(LinkLibraries_exe main.c)
target_link_libraries (LinkLibraries_exe PRIVATE $<$<LINK_LANGUAGE:C>:shared_C>)

add_executable(LinkLibraries_C_import main.c)
target_link_libraries (LinkLibraries_C_import PRIVATE import)
add_executable(LinkLibraries_CXX_import main.cxx)
target_link_libraries (LinkLibraries_CXX_import PRIVATE import)

add_executable(LinkLibraries_C_interface main.c)
target_link_libraries (LinkLibraries_C_interface PRIVATE interface)
add_executable(LinkLibraries_CXX_interface main.cxx)
target_link_libraries (LinkLibraries_CXX_interface PRIVATE interface)

add_executable(LinkLibraries_C_interface2 main.c)
target_link_libraries (LinkLibraries_C_interface2 PRIVATE interface2)
add_executable(LinkLibraries_CXX_interface2 main.cxx)
target_link_libraries (LinkLibraries_CXX_interface2 PRIVATE interface2)

add_executable(LinkLibraries_C_static main.c)
target_link_libraries (LinkLibraries_C_static PRIVATE static3)
add_executable(LinkLibraries_CXX_static main.cxx)
target_link_libraries (LinkLibraries_CXX_static PRIVATE static3)

# $<LINK_LANGUAGE:> change, by default, link language from C to CXX
# but because LINKER_LANGUAGE property is set, keep C as link language
add_executable(LinkLibraries_C_static_CXX main.c)
target_compile_definitions (LinkLibraries_C_static_CXX PRIVATE C_USE_CXX)
target_link_libraries (LinkLibraries_C_static_CXX PRIVATE $<$<LINK_LANGUAGE:C>:static_cxx>)
set_property(TARGET LinkLibraries_C_static_CXX PROPERTY LINKER_LANGUAGE C)
