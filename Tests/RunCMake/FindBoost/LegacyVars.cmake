find_package(Boost 1.70 COMPONENTS date_time python37 mpi_python2)

foreach(var Boost_FOUND Boost_INCLUDE_DIRS Boost_LIBRARY_DIRS Boost_LIBRARIES
  Boost_DATE_TIME_FOUND Boost_DATE_TIME_LIBRARY
  Boost_PYTHON37_FOUND Boost_PYTHON37_LIBRARY
  Boost_MPI_PYTHON2_FOUND Boost_MPI_PYTHON2_LIBRARY
  Boost_VERSION_MACRO Boost_VERSION_STRING Boost_VERSION Boost_LIB_VERSION
  Boost_MAJOR_VERSION Boost_MINOR_VERSION Boost_SUBMINOR_VERSION
)
  message(STATUS "${var}: ${${var}}")
endforeach()

foreach(cachevar Boost_INCLUDE_DIR
  Boost_DATE_TIME_LIBRARY_DEBUG Boost_DATE_TIME_LIBRARY_RELEASE
  Boost_PYTHON37_LIBRARY_DEBUG Boost_PYTHON37_LIBRARY_RELEASE
  Boost_MPI_PYTHON2_LIBRARY_DEBUG Boost_MPI_PYTHON2_LIBRARY_RELEASE
)
  unset(${cachevar})
  message(STATUS "${cachevar}: ${${cachevar}}")
endforeach()

foreach(lib Boost::headers Boost::date_time Boost::python Boost::mpi_python
  Boost::boost Boost::diagnostic_definitions Boost::disable_autolinking Boost::dynamic_linking
)
  if(NOT TARGET ${lib})
    message(FATAL_ERROR "Missing target ${lib}")
  endif()
endforeach()
