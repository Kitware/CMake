add_library(Boost::boost INTERFACE IMPORTED)
add_library(Boost::headers INTERFACE IMPORTED)
target_include_directories(Boost::headers INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

set(Boost_date_time_FOUND 1)
add_library(Boost::date_time UNKNOWN IMPORTED)
set_target_properties(Boost::date_time PROPERTIES
  IMPORTED_CONFIGURATIONS RELEASE
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_date_time.a"
  )

set(Boost_python37_FOUND 1)
add_library(Boost::python UNKNOWN IMPORTED)
set_target_properties(Boost::python PROPERTIES
  IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_python_release.a"
  IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_python.a"
  )
# Versioned target alias for compatibility (added by upstream BoostConfig).
add_library(Boost::python37 INTERFACE IMPORTED)
set_property(TARGET Boost::python37 APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::python)

set(Boost_mpi_python2_FOUND 1)
add_library(Boost::mpi_python UNKNOWN IMPORTED)
set_target_properties(Boost::mpi_python PROPERTIES
  IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_mpi_python.a"
  IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/lib/libboost_mpi_python.a"
  )
