
set(language "perl")

include (LegacyConfiguration.cmake)

if (WIN32)
  file (TO_CMAKE_PATH "$ENV{PATH}" perl_path)
  string (REPLACE ";" "$<SEMICOLON>" perl_path "${perl_path}")
  set (perl_env "PATH=$<TARGET_FILE_DIR:${SWIG_MODULE_example_REAL_NAME}>$<SEMICOLON>${perl_path}")
else()
  set (perl_env "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:${SWIG_MODULE_example_REAL_NAME}>")
endif()

add_custom_target (RunPerl
  COMMAND "${CMAKE_COMMAND}" -E env "${perl_env}"
  "${PERL_EXECUTABLE}" "-I$<TARGET_FILE_DIR:${SWIG_MODULE_example_REAL_NAME}>"
  "${CMAKE_CURRENT_SOURCE_DIR}/runme.pl"
  DEPENDS ${SWIG_MODULE_example_REAL_NAME})
