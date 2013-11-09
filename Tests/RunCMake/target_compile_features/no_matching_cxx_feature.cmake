
# Assume for testing that a compiler which supports gnuxx_typeof does not
# support msvcxx_sealed
if (";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";gnuxx_typeof;")
  set(feature msvcxx_sealed)
  if (";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";msvcxx_sealed;")
    message(SEND_ERROR "Compiler supports both gnuxx_typeof and msvcxx_sealed!")
  endif()
else()
  set(feature gnuxx_typeof)
endif()

message("CMAKE_CXX_COMPILE_FEATURES ${CMAKE_CXX_COMPILE_FEATURES}")

add_executable(main empty.cpp)

target_compile_features(main
  PRIVATE
    ${feature}
)
