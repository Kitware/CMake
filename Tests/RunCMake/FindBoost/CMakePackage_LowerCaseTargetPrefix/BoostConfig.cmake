# No boost::boost target and all targets start with lowercase letters
# Similar to https://github.com/boost-cmake/boost-cmake

add_library(boost::headers INTERFACE IMPORTED)
target_include_directories(boost::headers INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

set(Boost_date_time_FOUND 1)
add_library(boost::date_time UNKNOWN IMPORTED)
set_target_properties(boost::date_time PROPERTIES
  IMPORTED_CONFIGURATIONS RELEASE
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_date_time.a"
  )
set(Boost_python37_FOUND 1)
add_library(boost::python UNKNOWN IMPORTED)
set_target_properties(boost::python PROPERTIES
  IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_python_release.a"
  IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_python.a"
  )
set(Boost_mpi_python2_FOUND 1)
add_library(boost::mpi_python UNKNOWN IMPORTED)
set_target_properties(boost::mpi_python PROPERTIES
  IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_mpi_python.a"
  IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_mpi_python.a"
  )
