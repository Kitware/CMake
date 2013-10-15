try_compile
-----------

Try building some code.

::

  try_compile(RESULT_VAR <bindir> <srcdir>
              <projectName> [targetName] [CMAKE_FLAGS flags...]
              [OUTPUT_VARIABLE <var>])

Try building a project.  In this form, srcdir should contain a
complete CMake project with a CMakeLists.txt file and all sources.
The bindir and srcdir will not be deleted after this command is run.
Specify targetName to build a specific target instead of the 'all' or
'ALL_BUILD' target.

::

  try_compile(RESULT_VAR <bindir> <srcfile|SOURCES srcfile...>
              [CMAKE_FLAGS flags...]
              [COMPILE_DEFINITIONS flags...]
              [LINK_LIBRARIES libs...]
              [OUTPUT_VARIABLE <var>]
              [COPY_FILE <fileName> [COPY_FILE_ERROR <var>]])

Try building an executable from one or more source files.  In this
form the user need only supply one or more source files that include a
definition for 'main'.  CMake will create a CMakeLists.txt file to
build the source(s) as an executable.  Specify COPY_FILE to get a copy
of the linked executable at the given fileName and optionally
COPY_FILE_ERROR to capture any error.

In this version all files in bindir/CMakeFiles/CMakeTmp will be
cleaned automatically.  For debugging, --debug-trycompile can be
passed to cmake to avoid this clean.  However, multiple sequential
try_compile operations reuse this single output directory.  If you use
--debug-trycompile, you can only debug one try_compile call at a time.
The recommended procedure is to configure with cmake all the way
through once, then delete the cache entry associated with the
try_compile call of interest, and then re-run cmake again with
--debug-trycompile.

Some extra flags that can be included are, INCLUDE_DIRECTORIES,
LINK_DIRECTORIES, and LINK_LIBRARIES.  COMPILE_DEFINITIONS are
-Ddefinition that will be passed to the compile line.

The srcfile signature also accepts a LINK_LIBRARIES argument which may
contain a list of libraries or IMPORTED targets which will be linked
to in the generated project.  If LINK_LIBRARIES is specified as a
parameter to try_compile, then any LINK_LIBRARIES passed as
CMAKE_FLAGS will be ignored.

try_compile creates a CMakeList.txt file on the fly that looks like
this:

::

  add_definitions( <expanded COMPILE_DEFINITIONS from calling cmake>)
  include_directories(${INCLUDE_DIRECTORIES})
  link_directories(${LINK_DIRECTORIES})
  add_executable(cmTryCompileExec sources)
  target_link_libraries(cmTryCompileExec ${LINK_LIBRARIES})

In both versions of the command, if OUTPUT_VARIABLE is specified, then
the output from the build process is stored in the given variable.
The success or failure of the try_compile, i.e.  TRUE or FALSE
respectively, is returned in RESULT_VAR.  CMAKE_FLAGS can be used to
pass -DVAR:TYPE=VALUE flags to the cmake that is run during the build.
Set variable CMAKE_TRY_COMPILE_CONFIGURATION to choose a build
configuration.
