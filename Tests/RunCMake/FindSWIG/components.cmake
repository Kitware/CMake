# Note that 'perl' will not be found because it is not lowercase.
find_package(SWIG REQUIRED
  COMPONENTS java
  OPTIONAL_COMPONENTS python PERL)

message(STATUS "SWIG_VERSION='${SWIG_VERSION}'")
foreach(_lang java python PERL)
  message(STATUS "SWIG_${_lang}_FOUND=${SWIG_${_lang}_FOUND}")
endforeach()
