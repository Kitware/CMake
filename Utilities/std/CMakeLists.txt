
# To ensure maximum portability across various compilers and platforms
# deactivate any compiler extensions
set(CMAKE_CXX_EXTENSIONS FALSE)

# source files for CMake std library
set(SRCS cm/bits/string_view.cxx
         cm/memory
         cm/optional
         cm/shared_mutex
         cm/string_view
         cm/utility)

add_library(cmstd STATIC ${SRCS})
