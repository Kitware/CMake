enable_language(C)

add_executable(main ${CMAKE_CURRENT_BINARY_DIR}/main.c)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check-$<LOWER_CASE:$<CONFIG>>.cmake CONTENT "
set(check_pairs
  \"$<TARGET_FILE:main>|${CMAKE_CURRENT_BINARY_DIR}/main.c\"
  \"$<TARGET_FILE:main>|${CMAKE_CURRENT_BINARY_DIR}/main.h\"
  )
set(check_exes
  \"$<TARGET_FILE:main>\"
  )

if (check_step EQUAL 2)
  include(\"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/Makefile.cmake\")
  if (NOT CMAKE_DEPEND_INFO_FILES)
    set(RunCMake_TEST_FAILED \"Variable CMAKE_DEPEND_INFO_FILES not found.\")
  else()
    include(\"${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_DEPEND_INFO_FILES}\")
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
        if(NOT DEPENDS_CONTENT MATCHES \"\${OBJECT_FILE} *:.+main.c\"
            OR NOT DEPENDS_CONTENT MATCHES \"main.h\")
          set(RunCMake_TEST_FAILED \"Dependency file '\${TARGET_DEP_FILE}' badly generated.\")
        endif()
      endif()
    endif()
  endif()
endif()
")
