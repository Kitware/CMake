set(CMake_RUN_CLANG_TIDY ON CACHE BOOL "")
set(CMake_USE_CLANG_TIDY_MODULE ON CACHE BOOL "")
set(CMake_CLANG_TIDY_MODULE "$ENV{CI_PROJECT_DIR}/Utilities/ClangTidyModule/build/libcmake-clang-tidy-module.so" CACHE FILEPATH "")
set(CMake_CLANG_TIDY_EXPORT_FIXES_DIR "$ENV{CI_PROJECT_DIR}/.gitlab/clang-tidy-fixes" CACHE PATH "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora39_common.cmake")
