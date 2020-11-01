
# To ensure maximum portability across various compilers and platforms
# deactivate any compiler extensions
set(CMAKE_CXX_EXTENSIONS FALSE)

# source files for CMake std library
set(SRCS cm/bits/fs_path.cxx
         cm/bits/string_view.cxx
         cm/filesystem
         cm/memory
         cm/optional
         cm/shared_mutex
         cm/string_view
         cm/utility
         cmext/string_view)

add_library(cmstd STATIC ${SRCS})
