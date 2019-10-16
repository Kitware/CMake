target_precompile_headers
-------------------------

Add a list of header files to precompile.

.. code-block:: cmake

  target_precompile_headers(<target>
    <INTERFACE|PUBLIC|PRIVATE> [header1...]
    [<INTERFACE|PUBLIC|PRIVATE> [header2...] ...])

  target_precompile_headers(<target> REUSE_FROM <other_target>)

Adds header files to :prop_tgt:`PRECOMPILE_HEADERS` or
:prop_tgt:`INTERFACE_PRECOMPILE_HEADERS` target properties.

The second signature will reuse an already precompiled header file artefact
from another target. This is done by setting the
:prop_tgt:`PRECOMPILE_HEADERS_REUSE_FROM` to ``<other_target>`` value.
The ``<other_target>`` will become a dependency of ``<target>``.

.. note::

  The second signature will require the same set of compiler options,
  compiler flags, compiler definitions for both ``<target>``, and
  ``<other_target>``. Compilers (e.g. GCC) will issue a warning if the
  precompiled header file cannot be used (``-Winvalid-pch``).

Precompiling header files can speed up compilation by creating a partially
processed version of some header files, and then using that version during
compilations rather than repeatedly parsing the original headers.

The named ``<target>`` must have been created by a command such as
:command:`add_executable` or :command:`add_library` and must not be an
:ref:`ALIAS target <Alias Targets>`.

The ``INTERFACE``, ``PUBLIC`` and ``PRIVATE`` keywords are required to
specify the scope of the following arguments.  ``PRIVATE`` and ``PUBLIC``
items will populate the :prop_tgt:`PRECOMPILE_HEADERS` property of
``<target>``.  ``PUBLIC`` and ``INTERFACE`` items will populate the
:prop_tgt:`INTERFACE_PRECOMPILE_HEADERS` property of ``<target>``.
(:ref:`IMPORTED targets <Imported Targets>` only support ``INTERFACE`` items.)
Repeated calls for the same ``<target>`` append items in the order called.

Arguments to ``target_precompile_headers`` may use "generator expressions"
with the syntax ``$<...>``.
See the :manual:`cmake-generator-expressions(7)` manual for available
expressions.  See the :manual:`cmake-compile-features(7)` manual for
information on compile features and a list of supported compilers.
The ``$<COMPILE_LANGUAGE:...>`` generator expression is particularly
useful for specifying a language-specific header to precompile for
only one language (e.g. ``CXX`` and not ``C``).

Usage
^^^^^

.. code-block:: cmake

  target_precompile_headers(<target>
    PUBLIC
      project_header.h
      "$<$<COMPILE_LANGUAGE:CXX>:cxx_only.h>"
    PRIVATE
      [["other_header.h"]]
      <unordered_map>
  )

The list of header files is used to generate a header file named
``cmake_pch.h|xx`` which is used to generate the precompiled header file
(``.pch``, ``.gch``, ``.pchi``) artifact.  The ``cmake_pch.h|xx`` header
file will be force included (``-include`` for GCC, ``/FI`` for MSVC) to
all source files, so sources do not need to have ``#include "pch.h"``.

Header file names specified with angle brackets (e.g. ``<unordered_map>``) or
explicit double quotes (escaped for the :manual:`cmake-language(7)`,
e.g. ``[["other_header.h"]]``) will be treated as is, and include directories
must be available for the compiler to find them.  Other header file names
(e.g. ``project_header.h``) are interpreted as being relative to the current
source directory (e.g. :variable:`CMAKE_CURRENT_SOURCE_DIR`) and will be
included by absolute path.

See Also
^^^^^^^^

For disabling precompile headers for specific targets there is the
property :prop_tgt:`DISABLE_PRECOMPILE_HEADERS`.

For skipping certain source files there is the source file property
:prop_sf:`SKIP_PRECOMPILE_HEADERS`.
