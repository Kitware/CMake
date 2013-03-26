For more information about how to contribute modules to CMake, see this page:
http://www.itk.org/Wiki/CMake:Module_Maintainers

Note to authors of FindXxx.cmake files

We would like all FindXxx.cmake files to produce consistent variable names.

Please use the following consistent variable names for general use.

Xxx_INCLUDE_DIRS        The final set of include directories listed in one variable for use by client code.
                        This should not be a cache entry.
Xxx_LIBRARIES           The libraries to link against to use Xxx. These should include full paths.
                        This should not be a cache entry.
Xxx_DEFINITIONS         Definitions to use when compiling code that uses Xxx. This really shouldn't include options such
                        as (-DHAS_JPEG)that a client source-code file uses to decide whether to #include <jpeg.h>
Xxx_EXECUTABLE          Where to find the Xxx tool.
Xxx_Yyy_EXECUTABLE      Where to find the Yyy tool that comes with Xxx.
Xxx_LIBRARY_DIRS        Optionally, the final set of library directories listed in one variable for use by client code.
                        This should not be a cache entry.
Xxx_ROOT_DIR            Where to find the base directory of Xxx.
Xxx_VERSION_Yy          Expect Version Yy if true. Make sure at most one of these is ever true.
Xxx_WRAP_Yy             If False, do not try to use the relevant CMake wrapping command.
Xxx_Yy_FOUND            If False, optional Yy part of Xxx sytem is not available.
Xxx_FOUND               Set to false, or undefined, if we haven't found, or don't want to use Xxx.
Xxx_NOT_FOUND_MESSAGE   Should be set by config-files in the case that it has set Xxx_FOUND to FALSE.
                        The contained message will be printed by the find_package() command and by
                        find_package_handle_standard_args() to inform the user about the problem.
Xxx_RUNTIME_LIBRARY_DIRS Optionally, the runtime library search path for use when running an executable linked to
                         shared libraries.
                         The list should be used by user code to create the PATH on windows or LD_LIBRARY_PATH on unix.
                         This should not be a cache entry.
Xxx_VERSION_STRING      A human-readable string containing the version of the package found, if any.
Xxx_VERSION_MAJOR       The major version of the package found, if any.
Xxx_VERSION_MINOR       The minor version of the package found, if any.
Xxx_VERSION_PATCH       The patch version of the package found, if any.

You do not have to provide all of the above variables. You should provide Xxx_FOUND under most circumstances.
If Xxx is a library, then  Xxx_LIBRARIES, should also be defined, and Xxx_INCLUDE_DIRS should usually be
defined (I guess libm.a might be an exception)

The following names should not usually be used in CMakeLists.txt files, but they may be usefully modified in
users' CMake Caches to control stuff.

Xxx_LIBRARY             Name of Xxx Library. A User may set this and Xxx_INCLUDE_DIR to ignore to force non-use of Xxx.
Xxx_Yy_LIBRARY          Name of Yy library that is part of the Xxx system. It may or may not be required to use Xxx.
Xxx_INCLUDE_DIR         Where to find xxx.h, etc.  (Xxx_INCLUDE_PATH was considered bad because a path includes an
                        actual filename.)
Xxx_Yy_INCLUDE_DIR      Where to find xxx_yy.h, etc.

For tidiness's sake, try to keep as many options as possible out of the cache, leaving at least one option which can be
used to disable use of the module, or locate a not-found library (e.g. Xxx_ROOT_DIR).
For the same reason, mark most cache options as advanced.

If you need other commands to do special things then it should still begin with Xxx_. This gives a sort of namespace
effect and keeps things tidy for the user. You should put comments describing all the exported settings, plus
descriptions of any the users can use to control stuff.

You really should also provide backwards compatibility any old settings that were actually in use.
Make sure you comment them as deprecated, so that no-one starts using them.

To correctly document a module, create a comment block at the top with # comments.
There are three types of comments that can be in the block:

1. The brief description of the module, this is done by:
# - a small description

2. A paragraph of text.  This is done with all text that has a single
space between the # and the text.  To create a new paragraph, just
put a # with no text on the line.

3. A verbatim line.  This is done with two spaces between the # and the text.

For example:

# - This is a cool module
# This module does really cool stuff.
# It can do even more than you think.
#
# It even needs to paragraphs to tell you about it.
# And it defines the following variables:
#  VAR_COOL - this is great isn't it?
#  VAR_REALLY_COOL - cool right?
#

Test the documentation formatting by running "cmake --help-module FindXxx".
Edit the comments until the output of this command looks satisfactory.

To have a .cmake file in this directory NOT show up in the
modules documentation, you should start the file with a blank
line.

After the documentation, leave a *BLANK* line, and then add a
copyright and licence notice block like this one:

#=============================================================================
# Copyright 2009-2011 Your Name
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

The layout of the notice block is strictly enforced by the ModuleNotices test.
Only the year range and name may be changed freely.

A FindXxx.cmake module will typically be loaded by the command

  FIND_PACKAGE(Xxx [major[.minor[.patch[.tweak]]]] [EXACT]
               [QUIET] [[REQUIRED|COMPONENTS] [components...]])

If any version numbers are given to the command it will set the
following variables before loading the module:

  Xxx_FIND_VERSION       = full requested version string
  Xxx_FIND_VERSION_MAJOR = major version if requested, else 0
  Xxx_FIND_VERSION_MINOR = minor version if requested, else 0
  Xxx_FIND_VERSION_PATCH = patch version if requested, else 0
  Xxx_FIND_VERSION_TWEAK = tweak version if requested, else 0
  Xxx_FIND_VERSION_COUNT = number of version components, 0 to 4
  Xxx_FIND_VERSION_EXACT = true if EXACT option was given

If the find module supports versioning it should locate a version of
the package that is compatible with the version requested.  If a
compatible version of the package cannot be found the module should
not report success.  The version of the package found should be stored
in "Xxx_VERSION..." version variables documented by the module.

If the QUIET option is given to the command it will set the variable
Xxx_FIND_QUIETLY to true before loading the FindXxx.cmake module.  If
this variable is set the module should not complain about not being
able to find the package.  If the
REQUIRED option is given to the command it will set the variable
Xxx_FIND_REQUIRED to true before loading the FindXxx.cmake module.  If
this variable is set the module should issue a FATAL_ERROR if the
package cannot be found.
If neither the QUIET nor REQUIRED options are given then the
FindXxx.cmake module should look for the package and complain without
error if the module is not found.

FIND_PACKAGE() will set the variable CMAKE_FIND_PACKAGE_NAME to
contain the actual name of the package.

A package can provide sub-components.
Those components can be listed after the COMPONENTS (or REQUIRED)
or OPTIONAL_COMPONENTS keywords.  The set of all listed components will be
specified in a Xxx_FIND_COMPONENTS variable.
For each package-specific component, say Yyy, a variable Xxx_FIND_REQUIRED_Yyy
will be set to true if it listed after COMPONENTS and it will be set to false
if it was listed after OPTIONAL_COMPONENTS.
Using those variables a FindXxx.cmake module and also a XxxConfig.cmake package
configuration file can determine whether and which components have been requested,
and whether they were requested as required or as optional.
For each of the requested components a Xxx_Yyy_FOUND variable should be set
accordingly.
The per-package Xxx_FOUND variable should be only set to true if all requested
required components have been found. A missing optional component should not
keep the Xxx_FOUND variable from being set to true.
If the package provides Xxx_INCLUDE_DIRS and Xxx_LIBRARIES variables, the include
dirs and libraries for all components which were requested and which have been
found should be added to those two variables.

To get this behaviour you can use the FIND_PACKAGE_HANDLE_STANDARD_ARGS()
macro, as an example see FindJPEG.cmake.

For internal implementation, it's a generally accepted convention that variables starting with
underscore are for temporary use only. (variable starting with an underscore
are not intended as a reserved prefix).
