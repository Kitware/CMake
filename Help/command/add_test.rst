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

Arguments after COMMAND may use "generator expressions" with the syntax
"$<...>".  See the :manual:`cmake-generator-expressions(7)` manual for
available expressions.

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
