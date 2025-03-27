enable_language(C)

add_executable(main CompileDepends.c)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set(CODE_WITH_SPACE [[
add_executable(main2 ../CompileDepends.c)
target_include_directories(main2 PRIVATE ${CMAKE_BINARY_DIR})
]])
if(MAKE_SUPPORTS_SPACES)
  add_subdirectory("With Space")
  set(check_pairs_with_space "
    \"$<TARGET_FILE:main2>|${CMAKE_CURRENT_BINARY_DIR}/CompileDepends.h\"
  ")
  set(check_exes_with_space "
    \"$<TARGET_FILE:main2>\"
  ")
endif()

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
cmake_minimum_required(VERSION ${CMAKE_VERSION})
set(check_pairs
  \"$<TARGET_FILE:main>|${CMAKE_CURRENT_BINARY_DIR}/CompileDepends.h\"
  ${check_pairs_with_space}
  )
set(check_exes
  \"$<TARGET_FILE:main>\"
  ${check_exes_with_space}
  )

if (RunCMake_GENERATOR MATCHES \"Make\" AND check_step EQUAL 2)
  include(\"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Makefile.cmake\")
  if (NOT CMAKE_DEPEND_INFO_FILES)
    set(RunCMake_TEST_FAILED \"Variable CMAKE_DEPEND_INFO_FILES not found.\")
  else()
    list(FILTER CMAKE_DEPEND_INFO_FILES EXCLUDE REGEX main2)
    foreach(DEPEND_INFO_FILE IN LISTS CMAKE_DEPEND_INFO_FILES)
      include(\"${CMAKE_CURRENT_BINARY_DIR}/\${DEPEND_INFO_FILE}\")
      if (NOT CMAKE_DEPENDS_DEPENDENCY_FILES)
        set(RunCMake_TEST_FAILED \"Variable CMAKE_DEPENDS_DEPENDENCY_FILES not found.\")
      else()
        list(GET CMAKE_DEPENDS_DEPENDENCY_FILES 1 OBJECT_FILE)
        list(GET CMAKE_DEPENDS_DEPENDENCY_FILES 3 DEP_FILE)
        if (NOT EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/\${DEP_FILE}\")
          set(RunCMake_TEST_FAILED \"File \${DEP_FILE} not found.\")
        else()
          set (TARGET_DEP_FILE \"${CMAKE_CURRENT_BINARY_DIR}/\${DEP_FILE}\")
          cmake_path(REPLACE_FILENAME TARGET_DEP_FILE \"compiler_depend.make\")
          file(READ \"\${TARGET_DEP_FILE}\" DEPENDS_CONTENT)
          if (WIN32)
            string (REPLACE \"\\\\\" \"/\" DEPENDS_CONTENT \"\${DEPENDS_CONTENT}\")
            string (TOLOWER \"\${DEPENDS_CONTENT}\" DEPENDS_CONTENT)
            string (TOLOWER \"\${OBJECT_FILE}\" OBJECT_FILE)
          else()
            string(REPLACE \"\\\\ \" \" \" DEPENDS_CONTENT \"\${DEPENDS_CONTENT}\")
          endif()
          if(NOT DEPENDS_CONTENT MATCHES \"\${OBJECT_FILE} *:.+[Cc]ompile[Dd]epends.c\"
              OR NOT DEPENDS_CONTENT MATCHES \"[Cc]ompile[Dd]epends.h\")
            set(RunCMake_TEST_FAILED \"Dependency file badly generated:\n  \${TARGET_DEP_FILE}\")
          endif()
        endif()
      endif()
    endforeach()
  endif()
endif()

if (RunCMake_GENERATOR STREQUAL \"Ninja\" AND check_step EQUAL 2)
  execute_process(
    COMMAND \${RunCMake_MAKE_PROGRAM} -t deps
    WORKING_DIRECTORY \${RunCMake_TEST_BINARY_DIR}
    OUTPUT_VARIABLE deps
  )
  if(NOT deps MATCHES \"CompileDepends.c\" OR NOT deps MATCHES \"CompileDepends.h\")
    string(REPLACE deps \"\\n\" \"\\n  \" \"  \${deps}\")
    set(RunCMake_TEST_FAILED \"Dependencies not detected correctly:\\n\${deps}\")
  endif()
endif()
")
