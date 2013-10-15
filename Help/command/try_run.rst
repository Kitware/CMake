try_run
-------

Try compiling and then running some code.

::

  try_run(RUN_RESULT_VAR COMPILE_RESULT_VAR
          bindir srcfile [CMAKE_FLAGS <Flags>]
          [COMPILE_DEFINITIONS <flags>]
          [COMPILE_OUTPUT_VARIABLE comp]
          [RUN_OUTPUT_VARIABLE run]
          [OUTPUT_VARIABLE var]
          [ARGS <arg1> <arg2>...])

Try compiling a srcfile.  Return TRUE or FALSE for success or failure
in COMPILE_RESULT_VAR.  Then if the compile succeeded, run the
executable and return its exit code in RUN_RESULT_VAR.  If the
executable was built, but failed to run, then RUN_RESULT_VAR will be
set to FAILED_TO_RUN.  COMPILE_OUTPUT_VARIABLE specifies the variable
where the output from the compile step goes.  RUN_OUTPUT_VARIABLE
specifies the variable where the output from the running executable
goes.

For compatibility reasons OUTPUT_VARIABLE is still supported, which
gives you the output from the compile and run step combined.

Cross compiling issues

When cross compiling, the executable compiled in the first step
usually cannot be run on the build host.  try_run() checks the
CMAKE_CROSSCOMPILING variable to detect whether CMake is in
crosscompiling mode.  If that's the case, it will still try to compile
the executable, but it will not try to run the executable.  Instead it
will create cache variables which must be filled by the user or by
presetting them in some CMake script file to the values the executable
would have produced if it had been run on its actual target platform.
These variables are RUN_RESULT_VAR (explanation see above) and if
RUN_OUTPUT_VARIABLE (or OUTPUT_VARIABLE) was used, an additional cache
variable RUN_RESULT_VAR__COMPILE_RESULT_VAR__TRYRUN_OUTPUT.This is
intended to hold stdout and stderr from the executable.

In order to make cross compiling your project easier, use try_run only
if really required.  If you use try_run, use RUN_OUTPUT_VARIABLE (or
OUTPUT_VARIABLE) only if really required.  Using them will require
that when crosscompiling, the cache variables will have to be set
manually to the output of the executable.  You can also "guard" the
calls to try_run with if(CMAKE_CROSSCOMPILING) and provide an
easy-to-preset alternative for this case.

Set variable CMAKE_TRY_COMPILE_CONFIGURATION to choose a build
configuration.
