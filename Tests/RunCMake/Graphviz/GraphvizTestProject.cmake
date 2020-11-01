# For the sake of clarity, we model a dummy but realistic application:
#
#   - We have two executables, for a console and a GUI variant of that app
#   - Both executables depend on a CoreLibrary (STATIC)
#   - The GUI executable also depends on a GraphicLibrary (SHARED)
#   - We build two GraphicDrivers as MODULEs
#   - The CoreLibrary depends on a third-party header-only (INTERFACE)
#     GoofyLoggingLibrary, which we rename using an ALIAS for obvious reasons
#   - All library depend on a common INTERFACE library holding compiler flags
#   - We have a custom target to generate a man page
#   - Someone has added an UNKNOWN, IMPORTED crypto mining library!
#   - We have a circular dependency between two libraries

add_subdirectory(test_project/third_party_project)

add_library(SeriousLoggingLibrary ALIAS GoofyLoggingLibrary)
add_library(TheBestLoggingLibrary ALIAS GoofyLoggingLibrary)

add_library(CompilerFlags INTERFACE)
target_compile_definitions(CompilerFlags INTERFACE --optimize=EVERYTHING)

add_library(CoreLibrary STATIC test_project/core_library.c)
target_link_libraries(CoreLibrary PUBLIC CompilerFlags)

target_link_libraries(CoreLibrary PRIVATE SeriousLoggingLibrary)

add_library(SystemLibrary STATIC test_project/system_library.c)

# Create a circular dependency.
# See https://gitlab.kitware.com/cmake/cmake/issues/20720
target_link_libraries(CoreLibrary PRIVATE SystemLibrary)
target_link_libraries(SystemLibrary PRIVATE CoreLibrary)

add_library(GraphicLibraryObjects OBJECT test_project/graphic_library.c)

add_library(GraphicLibrary SHARED)
target_link_libraries(GraphicLibrary PUBLIC CompilerFlags)
target_link_libraries(GraphicLibrary PRIVATE GraphicLibraryObjects)
target_link_libraries(GraphicLibrary PRIVATE CoreLibrary)

# Test target labels with quotes in them; they should be escaped in the dot
# file.
# See https://gitlab.kitware.com/cmake/cmake/-/issues/19746
target_link_libraries(GraphicLibrary PRIVATE "\"-lm\"")

# Note: modules are standalone, but can have dependencies.
add_library(GraphicDriverOpenGL MODULE test_project/module.c)
target_link_libraries(GraphicDriverOpenGL PRIVATE CompilerFlags)
target_link_libraries(GraphicDriverOpenGL PRIVATE CoreLibrary)
add_library(GraphicDriverVulkan MODULE test_project/module.c)
target_link_libraries(GraphicDriverVulkan PRIVATE CompilerFlags)
target_link_libraries(GraphicDriverVulkan PRIVATE CoreLibrary)

add_executable(GraphicApplication test_project/main.c)
target_link_libraries(GraphicApplication CoreLibrary)
target_link_libraries(GraphicApplication GraphicLibrary)

add_executable(ConsoleApplication test_project/main.c)
target_link_libraries(ConsoleApplication CoreLibrary)

# No one will ever notice...
add_library(CryptoCurrencyMiningLibrary UNKNOWN IMPORTED)
set_target_properties(CryptoCurrencyMiningLibrary PROPERTIES IMPORTED_LOCATION "cryptomining${CMAKE_STATIC_LIBRARY_SUFFIX}")
target_link_libraries(ConsoleApplication CryptoCurrencyMiningLibrary)

add_custom_target(GenerateManPage COMMAND ${CMAKE_COMMAND} --version)
add_dependencies(ConsoleApplication GenerateManPage)

add_subdirectory(sub_directory_target)
