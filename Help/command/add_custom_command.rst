add_custom_command
------------------

Add a custom build rule to the generated build system.

There are two main signatures for add_custom_command The first
signature is for adding a custom command to produce an output.

::

  add_custom_command(OUTPUT output1 [output2 ...]
                     COMMAND command1 [ARGS] [args1...]
                     [COMMAND command2 [ARGS] [args2...] ...]
                     [MAIN_DEPENDENCY depend]
                     [DEPENDS [depends...]]
                     [IMPLICIT_DEPENDS <lang1> depend1
                                      [<lang2> depend2] ...]
                     [WORKING_DIRECTORY dir]
                     [COMMENT comment] [VERBATIM] [APPEND])

This defines a command to generate specified OUTPUT file(s).  A target
created in the same directory (CMakeLists.txt file) that specifies any
output of the custom command as a source file is given a rule to
generate the file using the command at build time.  Do not list the
output in more than one independent target that may build in parallel
or the two instances of the rule may conflict (instead use
add_custom_target to drive the command and make the other targets
depend on that one).  If an output name is a relative path it will be
interpreted relative to the build tree directory corresponding to the
current source directory.  Note that MAIN_DEPENDENCY is completely
optional and is used as a suggestion to visual studio about where to
hang the custom command.  In makefile terms this creates a new target
in the following form:

::

  OUTPUT: MAIN_DEPENDENCY DEPENDS
          COMMAND

If more than one command is specified they will be executed in order.
The optional ARGS argument is for backward compatibility and will be
ignored.

The second signature adds a custom command to a target such as a
library or executable.  This is useful for performing an operation
before or after building the target.  The command becomes part of the
target and will only execute when the target itself is built.  If the
target is already built, the command will not execute.

::

  add_custom_command(TARGET target
                     PRE_BUILD | PRE_LINK | POST_BUILD
                     COMMAND command1 [ARGS] [args1...]
                     [COMMAND command2 [ARGS] [args2...] ...]
                     [WORKING_DIRECTORY dir]
                     [COMMENT comment] [VERBATIM])

This defines a new command that will be associated with building the
specified target.  When the command will happen is determined by which
of the following is specified:

::

  PRE_BUILD - run before all other dependencies
  PRE_LINK - run after other dependencies
  POST_BUILD - run after the target has been built

Note that the PRE_BUILD option is only supported on Visual Studio 7 or
later.  For all other generators PRE_BUILD will be treated as
PRE_LINK.

If WORKING_DIRECTORY is specified the command will be executed in the
directory given.  If it is a relative path it will be interpreted
relative to the build tree directory corresponding to the current
source directory.  If COMMENT is set, the value will be displayed as a
message before the commands are executed at build time.  If APPEND is
specified the COMMAND and DEPENDS option values are appended to the
custom command for the first output specified.  There must have
already been a previous call to this command with the same output.
The COMMENT, WORKING_DIRECTORY, and MAIN_DEPENDENCY options are
currently ignored when APPEND is given, but may be used in the future.

If VERBATIM is given then all arguments to the commands will be
escaped properly for the build tool so that the invoked command
receives each argument unchanged.  Note that one level of escapes is
still used by the CMake language processor before add_custom_command
even sees the arguments.  Use of VERBATIM is recommended as it enables
correct behavior.  When VERBATIM is not given the behavior is platform
specific because there is no protection of tool-specific special
characters.

If the output of the custom command is not actually created as a file
on disk it should be marked as SYMBOLIC with
SET_SOURCE_FILES_PROPERTIES.

The IMPLICIT_DEPENDS option requests scanning of implicit dependencies
of an input file.  The language given specifies the programming
language whose corresponding dependency scanner should be used.
Currently only C and CXX language scanners are supported.  The
language has to be specified for every file in the IMPLICIT_DEPENDS
list.  Dependencies discovered from the scanning are added to those of
the custom command at build time.  Note that the IMPLICIT_DEPENDS
option is currently supported only for Makefile generators and will be
ignored by other generators.

If COMMAND specifies an executable target (created by ADD_EXECUTABLE)
it will automatically be replaced by the location of the executable
created at build time.  Additionally a target-level dependency will be
added so that the executable target will be built before any target
using this custom command.  However this does NOT add a file-level
dependency that would cause the custom command to re-run whenever the
executable is recompiled.

Arguments to COMMAND may use "generator expressions" with the syntax
"$<...>".  Generator expressions are evaluated during build system
generation to produce information specific to each build
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

Expressions with an implicit 'this' target:

::

  $<TARGET_PROPERTY:prop>   = The value of the property prop on the target on which the generator expression is evaluated.

References to target names in generator expressions imply target-level
dependencies, but NOT file-level dependencies.  List target names with
the DEPENDS option to add file dependencies.

The DEPENDS option specifies files on which the command depends.  If
any dependency is an OUTPUT of another custom command in the same
directory (CMakeLists.txt file) CMake automatically brings the other
custom command into the target in which this command is built.  If
DEPENDS is not specified the command will run whenever the OUTPUT is
missing; if the command does not actually create the OUTPUT then the
rule will always run.  If DEPENDS specifies any target (created by an
ADD_* command) a target-level dependency is created to make sure the
target is built before any target using this custom command.
Additionally, if the target is an executable or library a file-level
dependency is created to cause the custom command to re-run whenever
the target is recompiled.
