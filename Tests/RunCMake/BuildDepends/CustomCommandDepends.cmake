cmake_policy(SET CMP0116 NEW)
enable_language(C)

add_custom_command(OUTPUT main.c
  DEPFILE main.c.d
  COMMAND "${CMAKE_COMMAND}" -DINFILE=main.c.in -DOUTFILE=main.c -DDEPFILE=main.c.d
  -P "${CMAKE_CURRENT_SOURCE_DIR}/GenerateDepFile.cmake"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(mainc ALL DEPENDS main.c)

add_executable(main ${CMAKE_CURRENT_BINARY_DIR}/main.c)

set(CODE_WITH_SPACE [[
add_custom_command(OUTPUT main2.c
  DEPFILE main2.c.d
  COMMAND "${CMAKE_COMMAND}" -DINFILE=../main.c.in -DOUTFILE=main2.c -DDEPFILE=main2.c.d
  -P "${CMAKE_SOURCE_DIR}/GenerateDepFile.cmake"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(main2c ALL DEPENDS main2.c)

add_executable(main2 ${CMAKE_CURRENT_BINARY_DIR}/main2.c)
]])
if(MAKE_SUPPORTS_SPACES)
  add_subdirectory("With Space")
  set(check_pairs_with_space "
    \"$<TARGET_FILE:main2>|${CMAKE_CURRENT_BINARY_DIR}/main.c.in\"
    \"$<TARGET_FILE:main2>|${CMAKE_CURRENT_BINARY_DIR}/With Space/main2.c\"
  ")
  set(check_exes_with_space "
    \"$<TARGET_FILE:main2>\"
  ")
endif()

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
cmake_minimum_required(VERSION 3.19)
set(check_pairs
  \"$<TARGET_FILE:main>|${CMAKE_CURRENT_BINARY_DIR}/main.c.in\"
  \"$<TARGET_FILE:main>|${CMAKE_CURRENT_BINARY_DIR}/main.c\"
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
        list(LENGTH CMAKE_DEPENDS_DEPENDENCY_FILES DEPENDENCY_FILES_SIZE)
          math(EXPR STOP_INDEX \"\${DEPENDENCY_FILES_SIZE} - 1\")
        foreach(INDEX RANGE 0 \${STOP_INDEX} 4)
          math(EXPR OBJECT_INDEX \"\${INDEX} + 1\")
          math(EXPR FORMAT_INDEX \"\${INDEX} + 2\")
          math(EXPR DEP_INDEX \"\${INDEX} + 3\")
          list(GET CMAKE_DEPENDS_DEPENDENCY_FILES \${OBJECT_INDEX} OBJECT_FILE)
          list(GET CMAKE_DEPENDS_DEPENDENCY_FILES \${FORMAT_INDEX} DEP_FORMAT)
          list(GET CMAKE_DEPENDS_DEPENDENCY_FILES \${DEP_INDEX} DEP_FILE)
          if (NOT EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/\${DEP_FILE}\")
            set(RunCMake_TEST_FAILED \"File \${DEP_FILE} not found.\")
          else()
            cmake_path(APPEND TARGET_DEP_FILE \"${CMAKE_CURRENT_BINARY_DIR}\" \"\${DEPEND_INFO_FILE}\")
            cmake_path(REPLACE_FILENAME TARGET_DEP_FILE \"compiler_depend.make\")
            file(READ \"\${TARGET_DEP_FILE}\" DEPENDS_CONTENT)
            if (WIN32)
              string (REPLACE \"\\\\\" \"/\" DEPENDS_CONTENT \"\${DEPENDS_CONTENT}\")
              string (TOLOWER \"\${DEPENDS_CONTENT}\" DEPENDS_CONTENT)
              string (TOLOWER \"\${OBJECT_FILE}\" OBJECT_FILE)
            else()
              string(REPLACE \"\\\\ \" \" \" DEPENDS_CONTENT \"\${DEPENDS_CONTENT}\")
            endif()
            if(DEPEND_INFO_FILE MATCHES \"main\\\\.dir\")
              if (DEP_FORMAT STREQUAL \"gcc\" AND NOT DEPENDS_CONTENT MATCHES \"\${OBJECT_FILE} *:.+main.c\")
                set(RunCMake_TEST_FAILED \"Dependency file '\${TARGET_DEP_FILE}' badly generated:\\n\${DEPENDS_CONTENT}\")
              endif()
              if (DEP_FORMAT STREQUAL \"custom\" AND NOT DEPENDS_CONTENT MATCHES \"\${OBJECT_FILE} *:.+main.c.in\")
                set(RunCMake_TEST_FAILED \"Dependency file '\${TARGET_DEP_FILE}' badly generated:\\n\${DEPENDS_CONTENT}\")
              endif()
            else()
              if (NOT DEPENDS_CONTENT MATCHES \"\${OBJECT_FILE} *:.+main.c.in\")
                set(RunCMake_TEST_FAILED \"Dependency file '\${TARGET_DEP_FILE}' badly generated:\\n\${DEPENDS_CONTENT}\")
              endif()
            endif()
          endif()
        endforeach()
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
  if(NOT deps MATCHES \"main.c:.*main.c.in\")
    string(REPLACE deps \"\\n\" \"\\n  \" \"  \${deps}\")
    set(RunCMake_TEST_FAILED \"Dependencies not detected correctly:\\n\${deps}\")
  endif()
endif()
")
