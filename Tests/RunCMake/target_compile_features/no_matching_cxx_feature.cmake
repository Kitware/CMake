
if (NOT ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";gnuxx_typeof;"
    AND NOT ";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";msvcxx_sealed;" )
  # Simulate passing the test.
  message(SEND_ERROR
    "The compiler feature \"gnuxx_dummy\" is not known to compiler\n\"GNU\"\nversion 4.8.1."
  )
  return()
endif()

if (";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";gnuxx_typeof;")
  set(feature msvcxx_sealed)
  if (";${CMAKE_CXX_COMPILE_FEATURES};" MATCHES ";msvcxx_sealed;")
    # If a compiler supports both extensions, remove one of them.
    list(REMOVE_ITEM CMAKE_CXX_COMPILE_FEATURES msvcxx_sealed)
  endif()
else()
  set(feature gnuxx_typeof)
endif()

add_executable(main empty.cpp)

target_compile_features(main
  PRIVATE
    ${feature}
)
