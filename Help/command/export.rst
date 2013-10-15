export
------

Export targets from the build tree for use by outside projects.

::

  export(TARGETS [target1 [target2 [...]]] [NAMESPACE <namespace>]
         [APPEND] FILE <filename> [EXPORT_LINK_INTERFACE_LIBRARIES])

Create a file <filename> that may be included by outside projects to
import targets from the current project's build tree.  This is useful
during cross-compiling to build utility executables that can run on
the host platform in one project and then import them into another
project being compiled for the target platform.  If the NAMESPACE
option is given the <namespace> string will be prepended to all target
names written to the file.  If the APPEND option is given the
generated code will be appended to the file instead of overwriting it.
The EXPORT_LINK_INTERFACE_LIBRARIES keyword, if present, causes the
contents of the properties matching
(IMPORTED_)?LINK_INTERFACE_LIBRARIES(_<CONFIG>)? to be exported, when
policy CMP0022 is NEW.  If a library target is included in the export
but a target to which it links is not included the behavior is
unspecified.

The file created by this command is specific to the build tree and
should never be installed.  See the install(EXPORT) command to export
targets from an installation tree.

The properties set on the generated IMPORTED targets will have the
same values as the final values of the input TARGETS.

::

  export(PACKAGE <name>)

Store the current build directory in the CMake user package registry
for package <name>.  The find_package command may consider the
directory while searching for package <name>.  This helps dependent
projects find and use a package from the current project's build tree
without help from the user.  Note that the entry in the package
registry that this command creates works only in conjunction with a
package configuration file (<name>Config.cmake) that works with the
build tree.
