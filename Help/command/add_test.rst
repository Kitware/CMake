add_test
--------

Add a test to the project with the specified arguments.

::

  add_test(testname Exename arg1 arg2 ... )

If the ENABLE_TESTING command has been run, this command adds a test
target to the current directory.  If ENABLE_TESTING has not been run,
this command does nothing.  The tests are run by the testing subsystem
by executing Exename with the specified arguments.  Exename can be
either an executable built by this project or an arbitrary executable
on the system (like tclsh).  The test will be run with the current
working directory set to the CMakeList.txt files corresponding
directory in the binary tree.  Tests added using this signature do not
support generator expressions.



::

  add_test(NAME <name> [CONFIGURATIONS [Debug|Release|...]]
           [WORKING_DIRECTORY dir]
           COMMAND <command> [arg1 [arg2 ...]])

Add a test called <name>.  The test name may not contain spaces,
quotes, or other characters special in CMake syntax.  If COMMAND
specifies an executable target (created by add_executable) it will
automatically be replaced by the location of the executable created at
build time.  If a CONFIGURATIONS option is given then the test will be
executed only when testing under one of the named configurations.  If
a WORKING_DIRECTORY option is given then the test will be executed in
the given directory.

Arguments after COMMAND may use "generator expressions" with the
syntax "$<...>".  Generator expressions are evaluated during build
system generation to produce information specific to each build
configuration.  Valid expressions are:

::

  $<0:...>                  = empty string (ignores "...")
  $<1:...>                  = content of "..."
  $<CONFIG:cfg>             = '1' if config is "cfg", else '0'
  $<CONFIGURATION>          = configuration name
  $<BOOL:...>               = '1' if the '...' is true, else '0'
  $<STREQUAL:a,b>           = '1' if a is STREQUAL b, else '0'
  $<ANGLE-R>                = A literal '>'. Used to compare strings which contain a '>' for example.
  $<COMMA>                  = A literal ','. Used to compare strings which contain a ',' for example.
  $<SEMICOLON>              = A literal ';'. Used to prevent list expansion on an argument with ';'.
  $<JOIN:list,...>          = joins the list with the content of "..."
  $<TARGET_NAME:...>        = Marks ... as being the name of a target.  This is required if exporting targets to multiple dependent export sets.  The '...' must be a literal name of a target- it may not contain generator expressions.
  $<INSTALL_INTERFACE:...>  = content of "..." when the property is exported using install(EXPORT), and empty otherwise.
  $<BUILD_INTERFACE:...>    = content of "..." when the property is exported using export(), or when the target is used by another target in the same buildsystem. Expands to the empty string otherwise.
  $<PLATFORM_ID>            = The CMake-id of the platform   $<PLATFORM_ID:comp>       = '1' if the The CMake-id of the platform matches comp, otherwise '0'.
  $<C_COMPILER_ID>          = The CMake-id of the C compiler used.
  $<C_COMPILER_ID:comp>     = '1' if the CMake-id of the C compiler matches comp, otherwise '0'.
  $<CXX_COMPILER_ID>        = The CMake-id of the CXX compiler used.
  $<CXX_COMPILER_ID:comp>   = '1' if the CMake-id of the CXX compiler matches comp, otherwise '0'.
  $<VERSION_GREATER:v1,v2>  = '1' if v1 is a version greater than v2, else '0'.
  $<VERSION_LESS:v1,v2>     = '1' if v1 is a version less than v2, else '0'.
  $<VERSION_EQUAL:v1,v2>    = '1' if v1 is the same version as v2, else '0'.
  $<C_COMPILER_VERSION>     = The version of the C compiler used.
  $<C_COMPILER_VERSION:ver> = '1' if the version of the C compiler matches ver, otherwise '0'.
  $<CXX_COMPILER_VERSION>   = The version of the CXX compiler used.
  $<CXX_COMPILER_VERSION:ver> = '1' if the version of the CXX compiler matches ver, otherwise '0'.
  $<TARGET_FILE:tgt>        = main file (.exe, .so.1.2, .a)
  $<TARGET_LINKER_FILE:tgt> = file used to link (.a, .lib, .so)
  $<TARGET_SONAME_FILE:tgt> = file with soname (.so.3)

where "tgt" is the name of a target.  Target file expressions produce
a full path, but _DIR and _NAME versions can produce the directory and
file name components:

::

  $<TARGET_FILE_DIR:tgt>/$<TARGET_FILE_NAME:tgt>
  $<TARGET_LINKER_FILE_DIR:tgt>/$<TARGET_LINKER_FILE_NAME:tgt>
  $<TARGET_SONAME_FILE_DIR:tgt>/$<TARGET_SONAME_FILE_NAME:tgt>



::

  $<TARGET_PROPERTY:tgt,prop>   = The value of the property prop on the target tgt.

Note that tgt is not added as a dependency of the target this
expression is evaluated on.

::

  $<TARGET_POLICY:pol>          = '1' if the policy was NEW when the 'head' target was created, else '0'.  If the policy was not set, the warning message for the policy will be emitted.  This generator expression only works for a subset of policies.
  $<INSTALL_PREFIX>         = Content of the install prefix when the target is exported via INSTALL(EXPORT) and empty otherwise.

Boolean expressions:

::

  $<AND:?[,?]...>           = '1' if all '?' are '1', else '0'
  $<OR:?[,?]...>            = '0' if all '?' are '0', else '1'
  $<NOT:?>                  = '0' if '?' is '1', else '1'

where '?' is always either '0' or '1'.

Example usage:

::

  add_test(NAME mytest
           COMMAND testDriver --config $<CONFIGURATION>
                              --exe $<TARGET_FILE:myexe>)

This creates a test "mytest" whose command runs a testDriver tool
passing the configuration name and the full path to the executable
file produced by target "myexe".
