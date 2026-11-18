include(RunCMake)

run_cmake(COMPILE_DEFINITIONS)
run_cmake(COMPILE_FEATURES)
run_cmake(COMPILE_OPTIONS)
run_cmake(IMPORTED_GLOBAL)
run_cmake(INCLUDE_DIRECTORIES)
run_cmake(LINK_OPTIONS)
run_cmake(LINK_DIRECTORIES)
run_cmake(LINK_LIBRARIES)
run_cmake(SOURCES)
run_cmake(SOURCE_FILE)
run_cmake(TEST-invalid)
run_cmake(TYPE)
run_cmake(USER_PROP)
run_cmake(USER_PROP_INHERITED)

# Reuse the cache so the second configure exercises property entries that
# precede the corresponding INTERNAL value entry in CMakeCache.txt.
block()
  set(RunCMake_TEST_BINARY_DIR
    "${RunCMake_BINARY_DIR}/CachePropertyPersistsReload-build")
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_with_options(CachePropertyPersistsReload
    -DCachePropReloadStage=initialize)

  # MODIFIED is managed internally, so create its cache-file entries directly.
  # Match the serialization order: before INTERNAL values, after other values.
  set(cache_file "${RunCMake_TEST_BINARY_DIR}/CMakeCache.txt")
  if(EXISTS "${cache_file}") # The case may be excluded by RunCMake_TEST_FILTER.
    file(READ "${cache_file}" cache)
    foreach(name IN ITEMS Internal Empty)
      if(name STREQUAL "Empty")
        set(modified 0)
      else()
        set(modified 1)
      endif()
      string(REGEX REPLACE "(\n//[^\n]*\nCacheProp${name}:INTERNAL=)"
        "\nCacheProp${name}-MODIFIED:INTERNAL=${modified}\\1"
        cache "${cache}")
    endforeach()
    string(APPEND cache
      "\nCachePropString-MODIFIED:INTERNAL=1\nCachePropBool-MODIFIED:INTERNAL=1\n")
    file(WRITE "${cache_file}" "${cache}")
  endif()
  run_cmake_with_options(CachePropertyPersistsReload
    -DCachePropReloadStage=check)
endblock()

set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/TEST-build")
run_cmake(TEST)
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_command(TEST-test ${CMAKE_CTEST_COMMAND} -C Debug)

run_cmake(FILE_SET-invalid)
run_cmake(FILE_SET-TARGET-invalid)
run_cmake(FILE_SET-PROPERTY-missing)
run_cmake(FILE_SET)
