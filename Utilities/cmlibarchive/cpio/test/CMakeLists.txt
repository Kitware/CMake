############################################
#
# How to build bsdcpio_test
#
############################################
if(ENABLE_CPIO AND ENABLE_TEST)
  set(bsdcpio_test_SOURCES
    ../cmdline.c
    ../../libarchive_fe/err.c
    ../../libarchive_fe/pathmatch.c
    main.c
    test.h
    test_0.c
    test_basic.c
    test_cmdline.c
    test_format_newc.c
    test_gcpio_compat.c
    test_option_B_upper.c
    test_option_C_upper.c
    test_option_J_upper.c
    test_option_L_upper.c
    test_option_Z_upper.c
    test_option_a.c
    test_option_c.c
    test_option_d.c
    test_option_f.c
    test_option_help.c
    test_option_l.c
    test_option_lzma.c
    test_option_m.c
    test_option_t.c
    test_option_u.c
    test_option_version.c
    test_option_y.c
    test_option_z.c
    test_owner_parse.c
    test_passthrough_dotdot.c
    test_passthrough_reverse.c
    test_pathmatch.c
  )
  if(WIN32 AND NOT CYGWIN)
    list(APPEND bsdcpio_test_SOURCES ../cpio_windows.h)
  endif(WIN32 AND NOT CYGWIN)

  #
  # Generate the list.h
  #
  GENERATE_LIST_H(${CMAKE_CURRENT_BINARY_DIR}/list.h
    ${CMAKE_CURRENT_LIST_FILE} ${bsdcpio_test_SOURCES})
  set_property(DIRECTORY APPEND PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_BINARY_DIR})
  #
  # Register target
  #
  add_executable(bsdcpio_test ${bsdcpio_test_SOURCES})
  set_property(TARGET bsdcpio_test PROPERTY COMPILE_DEFINITIONS LIST_H)

  # add_test() for each separate test
  set(num 0)
  foreach(test ${bsdcpio_test_SOURCES})
    if(test MATCHES "^test_[^/]+[.]c$")
      string(REGEX REPLACE "^(test_[^/]+)[.]c$" "\\1" testname ${test})
      add_test("bsdcpio_${testname}" bsdcpio_test
        -v -p ${BSDCPIO} -r ${CMAKE_CURRENT_SOURCE_DIR} ${num})
      math(EXPR num "${num} + 1")
    endif(test MATCHES "^test_[^/]+[.]c$")
  endforeach(test)

  # Experimental new test handling
  add_custom_target(run_bsdcpio_test
    COMMAND bsdcpio_test -p ${BSDCPIO} -r ${CMAKE_CURRENT_SOURCE_DIR})
  add_dependencies(run_bsdcpio_test bsdcpio)
  add_dependencies(run_all_tests run_bsdcpio_test)
endif(ENABLE_CPIO AND ENABLE_TEST)

