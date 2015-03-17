target_link_libraries
---------------------

Link a target to given libraries.

::

  target_link_libraries(<target> [item1 [item2 [...]]]
                        [[debug|optimized|general] <item>] ...)

Specify libraries or flags to use when linking a given target.  The
named ``<target>`` must have been created in the current directory by a
command such as :command:`add_executable` or :command:`add_library`.  The
remaining arguments specify library names or flags.  Repeated calls for
the same ``<target>`` append items in the order called.

If a library name matches that of another target in the project a
dependency will automatically be added in the build system to make sure
the library being linked is up-to-date before the target links. Item names
starting with ``-``, but not ``-l`` or ``-framework``, are treated as
linker flags.  Note that such flags will be treated like any other library
link item for purposes of transitive dependencies, so they are generally
safe to specify only as private link items that will not propagate to
dependents of ``<target>``.

A ``debug``, ``optimized``, or ``general`` keyword indicates that the
library immediately following it is to be used only for the
corresponding build configuration.  The ``debug`` keyword corresponds to
the Debug configuration (or to configurations named in the
:prop_gbl:`DEBUG_CONFIGURATIONS` global property if it is set).  The
``optimized`` keyword corresponds to all other configurations.  The
``general`` keyword corresponds to all configurations, and is purely
optional (assumed if omitted).  Higher granularity may be achieved for
per-configuration rules by creating and linking to
:ref:`IMPORTED library targets <Imported Targets>`.

Library dependencies are transitive by default with this signature.
When this target is linked into another target then the libraries
linked to this target will appear on the link line for the other
target too.  This transitive "link interface" is stored in the
:prop_tgt:`INTERFACE_LINK_LIBRARIES` target property and may be overridden
by setting the property directly.  When :policy:`CMP0022` is not set to
``NEW``, transitive linking is built in but may be overridden by the
:prop_tgt:`LINK_INTERFACE_LIBRARIES` property.  Calls to other signatures
of this command may set the property making any libraries linked
exclusively by this signature private.

CMake will also propagate :ref:`usage requirements <Target Usage Requirements>`
from linked library targets.  Usage requirements of dependencies affect
compilation of sources in the ``<target>``.

.. |INTERFACE_PROPERTY_LINK| replace:: :prop_tgt:`INTERFACE_LINK_LIBRARIES`
.. include:: /include/INTERFACE_LINK_LIBRARIES_WARNING.txt

If an ``<item>`` is a library in a Mac OX framework, the ``Headers``
directory of the framework will also be processed as a
:ref:`usage requirement <Target Usage Requirements>`.  This has the same
effect as passing the framework directory as an include directory.

--------------------------------------------------------------------------

::

  target_link_libraries(<target>
                      <PRIVATE|PUBLIC|INTERFACE> <lib> ...
                      [<PRIVATE|PUBLIC|INTERFACE> <lib> ... ] ...])

The ``PUBLIC``, ``PRIVATE`` and ``INTERFACE`` keywords can be used to
specify both the link dependencies and the link interface in one command.
Libraries and targets following ``PUBLIC`` are linked to, and are made
part of the link interface.  Libraries and targets following ``PRIVATE``
are linked to, but are not made part of the link interface.  Libraries
following ``INTERFACE`` are appended to the link interface and are not
used for linking ``<target>``.

--------------------------------------------------------------------------

::

  target_link_libraries(<target> LINK_INTERFACE_LIBRARIES
                        [[debug|optimized|general] <lib>] ...)

The ``LINK_INTERFACE_LIBRARIES`` mode appends the libraries to the
:prop_tgt:`INTERFACE_LINK_LIBRARIES` target property instead of using them
for linking.  If policy :policy:`CMP0022` is not ``NEW``, then this mode
also appends libraries to the :prop_tgt:`LINK_INTERFACE_LIBRARIES` and its
per-configuration equivalent.

This signature is for compatibility only.  Prefer the ``INTERFACE`` mode
instead.

Libraries specified as ``debug`` are wrapped in a generator expression to
correspond to debug builds.  If policy :policy:`CMP0022` is
not ``NEW``, the libraries are also appended to the
:prop_tgt:`LINK_INTERFACE_LIBRARIES_DEBUG <LINK_INTERFACE_LIBRARIES_<CONFIG>>`
property (or to the properties corresponding to configurations listed in
the :prop_gbl:`DEBUG_CONFIGURATIONS` global property if it is set).
Libraries specified as ``optimized`` are appended to the
:prop_tgt:`INTERFACE_LINK_LIBRARIES` property.  If policy :policy:`CMP0022`
is not ``NEW``, they are also appended to the
:prop_tgt:`LINK_INTERFACE_LIBRARIES` property.  Libraries specified as
``general`` (or without any keyword) are treated as if specified for both
``debug`` and ``optimized``.

--------------------------------------------------------------------------

::

  target_link_libraries(<target>
                        <LINK_PRIVATE|LINK_PUBLIC>
                          [[debug|optimized|general] <lib>] ...
                        [<LINK_PRIVATE|LINK_PUBLIC>
                          [[debug|optimized|general] <lib>] ...])

The ``LINK_PUBLIC`` and ``LINK_PRIVATE`` modes can be used to specify both
the link dependencies and the link interface in one command.

This signature is for compatibility only.  Prefer the ``PUBLIC`` or
``PRIVATE`` keywords instead.

Libraries and targets following ``LINK_PUBLIC`` are linked to, and are
made part of the :prop_tgt:`INTERFACE_LINK_LIBRARIES`.  If policy
:policy:`CMP0022` is not ``NEW``, they are also made part of the
:prop_tgt:`LINK_INTERFACE_LIBRARIES`.  Libraries and targets following
``LINK_PRIVATE`` are linked to, but are not made part of the
:prop_tgt:`INTERFACE_LINK_LIBRARIES` (or :prop_tgt:`LINK_INTERFACE_LIBRARIES`).

The library dependency graph is normally acyclic (a DAG), but in the case
of mutually-dependent ``STATIC`` libraries CMake allows the graph to
contain cycles (strongly connected components).  When another target links
to one of the libraries, CMake repeats the entire connected component.
For example, the code

.. code-block:: cmake

  add_library(A STATIC a.c)
  add_library(B STATIC b.c)
  target_link_libraries(A B)
  target_link_libraries(B A)
  add_executable(main main.c)
  target_link_libraries(main A)

links ``main`` to ``A B A B``.  While one repetition is usually
sufficient, pathological object file and symbol arrangements can require
more.  One may handle such cases by manually repeating the component in
the last ``target_link_libraries`` call.  However, if two archives are
really so interdependent they should probably be combined into a single
archive.

Arguments to target_link_libraries may use "generator expressions"
with the syntax ``$<...>``.  Note however, that generator expressions
will not be used in OLD handling of :policy:`CMP0003` or :policy:`CMP0004`.
See the :manual:`cmake-generator-expressions(7)` manual for available
expressions.  See the :manual:`cmake-buildsystem(7)` manual for more on
defining buildsystem properties.
