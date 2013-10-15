target_link_libraries
---------------------

Link a target to given libraries.

::

  target_link_libraries(<target> [item1 [item2 [...]]]
                        [[debug|optimized|general] <item>] ...)

Specify libraries or flags to use when linking a given target.  The
named <target> must have been created in the current directory by a
command such as add_executable or add_library.  The remaining
arguments specify library names or flags.  Repeated calls for the same
<target> append items in the order called.

If a library name matches that of another target in the project a
dependency will automatically be added in the build system to make
sure the library being linked is up-to-date before the target links.
Item names starting with '-', but not '-l' or '-framework', are
treated as linker flags.

A "debug", "optimized", or "general" keyword indicates that the
library immediately following it is to be used only for the
corresponding build configuration.  The "debug" keyword corresponds to
the Debug configuration (or to configurations named in the
DEBUG_CONFIGURATIONS global property if it is set).  The "optimized"
keyword corresponds to all other configurations.  The "general"
keyword corresponds to all configurations, and is purely optional
(assumed if omitted).  Higher granularity may be achieved for
per-configuration rules by creating and linking to IMPORTED library
targets.  See the IMPORTED mode of the add_library command for more
information.

Library dependencies are transitive by default.  When this target is
linked into another target then the libraries linked to this target
will appear on the link line for the other target too.  See the
INTERFACE_LINK_LIBRARIES target property to override the set of
transitive link dependencies for a target.  Calls to other signatures
of this command may set the property making any libraries linked
exclusively by this signature private.

CMake will also propagate "usage requirements" from linked library
targets.  Usage requirements affect compilation of sources in the
<target>.  They are specified by properties defined on linked targets.
During generation of the build system, CMake integrates usage
requirement property values with the corresponding build properties
for <target>:

::

 INTERFACE_COMPILE_DEFINITONS: Appends to COMPILE_DEFINITONS
 INTERFACE_INCLUDE_DIRECTORIES: Appends to INCLUDE_DIRECTORIES
 INTERFACE_POSITION_INDEPENDENT_CODE: Sets POSITION_INDEPENDENT_CODE
   or checked for consistency with existing value



If an <item> is a library in a Mac OX framework, the Headers directory
of the framework will also be processed as a "usage requirement".
This has the same effect as passing the framework directory as an
include directory.  target_link_libraries(<target>

::

                      <PRIVATE|PUBLIC|INTERFACE> <lib> ...
                      [<PRIVATE|PUBLIC|INTERFACE> <lib> ... ] ...])

The PUBLIC, PRIVATE and INTERFACE keywords can be used to specify both
the link dependencies and the link interface in one command.
Libraries and targets following PUBLIC are linked to, and are made
part of the link interface.  Libraries and targets following PRIVATE
are linked to, but are not made part of the link interface.  Libraries
following INTERFACE are appended to the link interface and are not
used for linking <target>.

::

  target_link_libraries(<target> LINK_INTERFACE_LIBRARIES
                        [[debug|optimized|general] <lib>] ...)

The LINK_INTERFACE_LIBRARIES mode appends the libraries to the
INTERFACE_LINK_LIBRARIES target property instead of using them for
linking.  If policy CMP0022 is not NEW, then this mode also appends
libraries to the LINK_INTERFACE_LIBRARIES and its per-configuration
equivalent.  This signature is for compatibility only.  Prefer the
INTERFACE mode instead.  Libraries specified as "debug" are wrapped in
a generator expression to correspond to debug builds.  If policy
CMP0022 is not NEW, the libraries are also appended to the
LINK_INTERFACE_LIBRARIES_DEBUG property (or to the properties
corresponding to configurations listed in the DEBUG_CONFIGURATIONS
global property if it is set).  Libraries specified as "optimized" are
appended to the INTERFACE_LINK_LIBRARIES property.  If policy CMP0022
is not NEW, they are also appended to the LINK_INTERFACE_LIBRARIES
property.  Libraries specified as "general" (or without any keyword)
are treated as if specified for both "debug" and "optimized".

::

  target_link_libraries(<target>
                        <LINK_PRIVATE|LINK_PUBLIC>
                          [[debug|optimized|general] <lib>] ...
                        [<LINK_PRIVATE|LINK_PUBLIC>
                          [[debug|optimized|general] <lib>] ...])

The LINK_PUBLIC and LINK_PRIVATE modes can be used to specify both the
link dependencies and the link interface in one command.  This
signature is for compatibility only.  Prefer the PUBLIC or PRIVATE
keywords instead.  Libraries and targets following LINK_PUBLIC are
linked to, and are made part of the INTERFACE_LINK_LIBRARIES.  If
policy CMP0022 is not NEW, they are also made part of the
LINK_INTERFACE_LIBRARIES.  Libraries and targets following
LINK_PRIVATE are linked to, but are not made part of the
INTERFACE_LINK_LIBRARIES (or LINK_INTERFACE_LIBRARIES).

The library dependency graph is normally acyclic (a DAG), but in the
case of mutually-dependent STATIC libraries CMake allows the graph to
contain cycles (strongly connected components).  When another target
links to one of the libraries CMake repeats the entire connected
component.  For example, the code

::

  add_library(A STATIC a.c)
  add_library(B STATIC b.c)
  target_link_libraries(A B)
  target_link_libraries(B A)
  add_executable(main main.c)
  target_link_libraries(main A)

links 'main' to 'A B A B'.  (While one repetition is usually
sufficient, pathological object file and symbol arrangements can
require more.  One may handle such cases by manually repeating the
component in the last target_link_libraries call.  However, if two
archives are really so interdependent they should probably be combined
into a single archive.)

Arguments to target_link_libraries may use "generator expressions"
with the syntax "$<...>".  Note however, that generator expressions
will not be used in OLD handling of CMP0003 or CMP0004.

Generator expressions are evaluated during build system generation to
produce information specific to each build configuration.  Valid
expressions are:

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
