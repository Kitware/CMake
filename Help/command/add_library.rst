add_library
-----------

Add a library to the project using the specified source files.

::

  add_library(<name> [STATIC | SHARED | MODULE]
              [EXCLUDE_FROM_ALL]
              source1 source2 ... sourceN)

Adds a library target called <name> to be built from the source files
listed in the command invocation.  The <name> corresponds to the
logical target name and must be globally unique within a project.  The
actual file name of the library built is constructed based on
conventions of the native platform (such as lib<name>.a or
<name>.lib).

STATIC, SHARED, or MODULE may be given to specify the type of library
to be created.  STATIC libraries are archives of object files for use
when linking other targets.  SHARED libraries are linked dynamically
and loaded at runtime.  MODULE libraries are plugins that are not
linked into other targets but may be loaded dynamically at runtime
using dlopen-like functionality.  If no type is given explicitly the
type is STATIC or SHARED based on whether the current value of the
variable BUILD_SHARED_LIBS is true.  For SHARED and MODULE libraries
the POSITION_INDEPENDENT_CODE target property is set to TRUE
automatically.

By default the library file will be created in the build tree
directory corresponding to the source tree directory in which the
command was invoked.  See documentation of the
ARCHIVE_OUTPUT_DIRECTORY, LIBRARY_OUTPUT_DIRECTORY, and
RUNTIME_OUTPUT_DIRECTORY target properties to change this location.
See documentation of the OUTPUT_NAME target property to change the
<name> part of the final file name.

If EXCLUDE_FROM_ALL is given the corresponding property will be set on
the created target.  See documentation of the EXCLUDE_FROM_ALL target
property for details.

The add_library command can also create IMPORTED library targets using
this signature:

::

  add_library(<name> <SHARED|STATIC|MODULE|UNKNOWN> IMPORTED
              [GLOBAL])

An IMPORTED library target references a library file located outside
the project.  No rules are generated to build it.  The target name has
scope in the directory in which it is created and below, but the
GLOBAL option extends visibility.  It may be referenced like any
target built within the project.  IMPORTED libraries are useful for
convenient reference from commands like target_link_libraries.
Details about the imported library are specified by setting properties
whose names begin in "IMPORTED_".  The most important such property is
IMPORTED_LOCATION (and its per-configuration version
IMPORTED_LOCATION_<CONFIG>) which specifies the location of the main
library file on disk.  See documentation of the IMPORTED_* properties
for more information.

The signature

::

  add_library(<name> OBJECT <src>...)

creates a special "object library" target.  An object library compiles
source files but does not archive or link their object files into a
library.  Instead other targets created by add_library or
add_executable may reference the objects using an expression of the
form $<TARGET_OBJECTS:objlib> as a source, where "objlib" is the
object library name.  For example:

::

  add_library(... $<TARGET_OBJECTS:objlib> ...)
  add_executable(... $<TARGET_OBJECTS:objlib> ...)

will include objlib's object files in a library and an executable
along with those compiled from their own sources.  Object libraries
may contain only sources (and headers) that compile to object files.
They may contain custom commands generating such sources, but not
PRE_BUILD, PRE_LINK, or POST_BUILD commands.  Object libraries cannot
be imported, exported, installed, or linked.  Some native build
systems may not like targets that have only object files, so consider
adding at least one real source file to any target that references
$<TARGET_OBJECTS:objlib>.

The signature

::

  add_library(<name> ALIAS <target>)

creates an alias, such that <name> can be used to refer to <target> in
subsequent commands.  The <name> does not appear in the generated
buildsystem as a make target.  The <target> may not be an IMPORTED
target or an ALIAS.  Alias targets can be used as linkable targets,
targets to read properties from.  They can also be tested for
existance with the regular if(TARGET) subcommand.  The <name> may not
be used to modify properties of <target>, that is, it may not be used
as the operand of set_property, set_target_properties,
target_link_libraries etc.  An ALIAS target may not be installed of
exported.

The signature

::

  add_library(<name> INTERFACE)

creates an interface target.  An interface target does not directly
create build output, though it may have properties set on it and it
may be installed, exported and imported.  Typically the INTERFACE_*
properties are populated on the interface target using the
set_property(), target_link_libraries(), target_include_directories()
and target_compile_defintions() commands, and then it is used as an
argument to target_link_libraries() like any other target.
