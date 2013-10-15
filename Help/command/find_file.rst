find_file
---------

Find the full path to a file.

::

   find_file(<VAR> name1 [path1 path2 ...])

This is the short-hand signature for the command that is sufficient in
many cases.  It is the same as find_file(<VAR> name1 [PATHS path1
path2 ...])

::

   find_file(
             <VAR>
             name | NAMES name1 [name2 ...]
             [HINTS path1 [path2 ... ENV var]]
             [PATHS path1 [path2 ... ENV var]]
             [PATH_SUFFIXES suffix1 [suffix2 ...]]
             [DOC "cache documentation string"]
             [NO_DEFAULT_PATH]
             [NO_CMAKE_ENVIRONMENT_PATH]
             [NO_CMAKE_PATH]
             [NO_SYSTEM_ENVIRONMENT_PATH]
             [NO_CMAKE_SYSTEM_PATH]
             [CMAKE_FIND_ROOT_PATH_BOTH |
              ONLY_CMAKE_FIND_ROOT_PATH |
              NO_CMAKE_FIND_ROOT_PATH]
            )

This command is used to find a full path to named file.  A cache entry
named by <VAR> is created to store the result of this command.  If the
full path to a file is found the result is stored in the variable and
the search will not be repeated unless the variable is cleared.  If
nothing is found, the result will be <VAR>-NOTFOUND, and the search
will be attempted again the next time find_file is invoked with the
same variable.  The name of the full path to a file that is searched
for is specified by the names listed after the NAMES argument.
Additional search locations can be specified after the PATHS argument.
If ENV var is found in the HINTS or PATHS section the environment
variable var will be read and converted from a system environment
variable to a cmake style list of paths.  For example ENV PATH would
be a way to list the system path variable.  The argument after DOC
will be used for the documentation string in the cache.  PATH_SUFFIXES
specifies additional subdirectories to check below each search path.

If NO_DEFAULT_PATH is specified, then no additional paths are added to
the search.  If NO_DEFAULT_PATH is not specified, the search process
is as follows:

1.  Search paths specified in cmake-specific cache variables.  These
are intended to be used on the command line with a -DVAR=value.  This
can be skipped if NO_CMAKE_PATH is passed.

::

   <prefix>/include/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   <prefix>/include for each <prefix> in CMAKE_PREFIX_PATH
   CMAKE_INCLUDE_PATH
   CMAKE_FRAMEWORK_PATH

2.  Search paths specified in cmake-specific environment variables.
These are intended to be set in the user's shell configuration.  This
can be skipped if NO_CMAKE_ENVIRONMENT_PATH is passed.

::

   <prefix>/include/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   <prefix>/include for each <prefix> in CMAKE_PREFIX_PATH
   CMAKE_INCLUDE_PATH
   CMAKE_FRAMEWORK_PATH

3.  Search the paths specified by the HINTS option.  These should be
paths computed by system introspection, such as a hint provided by the
location of another item already found.  Hard-coded guesses should be
specified with the PATHS option.

4.  Search the standard system environment variables.  This can be
skipped if NO_SYSTEM_ENVIRONMENT_PATH is an argument.

::

   PATH
   INCLUDE

5.  Search cmake variables defined in the Platform files for the
current system.  This can be skipped if NO_CMAKE_SYSTEM_PATH is
passed.

::

   <prefix>/include/<arch> if CMAKE_LIBRARY_ARCHITECTURE is set, and
   <prefix>/include for each <prefix> in CMAKE_SYSTEM_PREFIX_PATH
   CMAKE_SYSTEM_INCLUDE_PATH
   CMAKE_SYSTEM_FRAMEWORK_PATH

6.  Search the paths specified by the PATHS option or in the
short-hand version of the command.  These are typically hard-coded
guesses.

On Darwin or systems supporting OS X Frameworks, the cmake variable
CMAKE_FIND_FRAMEWORK can be set to empty or one of the following:

::

   "FIRST"  - Try to find frameworks before standard
              libraries or headers. This is the default on Darwin.
   "LAST"   - Try to find frameworks after standard
              libraries or headers.
   "ONLY"   - Only try to find frameworks.
   "NEVER" - Never try to find frameworks.

On Darwin or systems supporting OS X Application Bundles, the cmake
variable CMAKE_FIND_APPBUNDLE can be set to empty or one of the
following:

::

   "FIRST"  - Try to find application bundles before standard
              programs. This is the default on Darwin.
   "LAST"   - Try to find application bundles after standard
              programs.
   "ONLY"   - Only try to find application bundles.
   "NEVER" - Never try to find application bundles.

The CMake variable CMAKE_FIND_ROOT_PATH specifies one or more
directories to be prepended to all other search directories.  This
effectively "re-roots" the entire search under given locations.  By
default it is empty.  It is especially useful when cross-compiling to
point to the root directory of the target environment and CMake will
search there too.  By default at first the directories listed in
CMAKE_FIND_ROOT_PATH and then the non-rooted directories will be
searched.  The default behavior can be adjusted by setting
CMAKE_FIND_ROOT_PATH_MODE_INCLUDE.  This behavior can be manually
overridden on a per-call basis.  By using CMAKE_FIND_ROOT_PATH_BOTH
the search order will be as described above.  If
NO_CMAKE_FIND_ROOT_PATH is used then CMAKE_FIND_ROOT_PATH will not be
used.  If ONLY_CMAKE_FIND_ROOT_PATH is used then only the re-rooted
directories will be searched.

The default search order is designed to be most-specific to
least-specific for common use cases.  Projects may override the order
by simply calling the command multiple times and using the NO_*
options:

::

   find_file(<VAR> NAMES name PATHS paths... NO_DEFAULT_PATH)
   find_file(<VAR> NAMES name)

Once one of the calls succeeds the result variable will be set and
stored in the cache so that no call will search again.
