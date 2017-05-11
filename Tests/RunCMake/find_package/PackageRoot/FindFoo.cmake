find_file(FOO_TEST_FILE_FOO foo.h)
find_path(FOO_TEST_PATH_FOO foo.h)
find_program(FOO_TEST_PROG_FOO foo.exe)

if ("Bar" IN_LIST Foo_FIND_COMPONENTS)
  find_package(Bar)
endif ()
