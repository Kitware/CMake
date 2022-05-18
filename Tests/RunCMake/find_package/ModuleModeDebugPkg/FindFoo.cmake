find_program(FOO_EXE NAMES ModuleModeDebugPkgFooExe NO_DEFAULT_PATH)
find_library(FOO_LIB NAMES ModuleModeDebugPkgFooLib NO_DEFAULT_PATH)
find_path(FOO_PATH NAMES ModuleModeDebugPkgFoo.h NO_DEFAULT_PATH)
find_file(FOO_FILE NAMES ModuleModeDebugPkgFoo.h NO_DEFAULT_PATH)
find_package(Bar) # not included
find_package(Zot NO_MODULE NO_DEFAULT_PATH) # is included
