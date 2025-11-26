enable_language(C)

include(CheckTypeSize)
check_type_size(int SIZEOF_INT)
message(STATUS "SIZEOF_INT='${SIZEOF_INT}'")
message(STATUS "HAVE_SIZEOF_INT='${HAVE_SIZEOF_INT}'")
