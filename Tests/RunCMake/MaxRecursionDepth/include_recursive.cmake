message("${x}")
math(EXPR x "${x} + 1")
include("${CMAKE_CURRENT_LIST_FILE}")
