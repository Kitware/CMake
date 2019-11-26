
# source files for CMake std library
set(SRCS cm/bits/string_view.cxx
         cm/memory
         cm/optional
         cm/shared_mutex
         cm/string_view
         cm/utility)

add_library(cmstd STATIC ${SRCS})
