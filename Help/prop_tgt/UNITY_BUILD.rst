UNITY_BUILD
-----------

Should the target source files be processed into batches for
faster compilation. This feature is known as "Unity build",
or "Jumbo build".

The ``C`` and ``CXX`` source files are grouped separately.

This property is initialized by the value of the
:variable:`CMAKE_UNITY_BUILD` variable if it is set when
a target is created.

.. note::

  It's not recommended to directly set :prop_tgt:`UNITY_BUILD`
  to ``ON``, but to instead set :variable:`CMAKE_UNITY_BUILD` from
  the command line.  However, it IS recommended to set
  :prop_tgt:`UNITY_BUILD` to ``OFF`` if you need to ensure that a
  target doesn't get a unity build.

The batch size can be specified by setting
:prop_tgt:`UNITY_BUILD_BATCH_SIZE`.

The batching of source files is done by adding new sources files
which will ``#include`` the source files, and exclude them from
building by setting :prop_sf:`HEADER_FILE_ONLY` to ``ON``.

.. note::

  Marking the original sources with :prop_sf:`HEADER_FILE_ONLY`
  is considered an implementation detail that may change in the
  future because it does not work well in combination with
  the :variable:`CMAKE_EXPORT_COMPILE_COMMANDS` variable.

ODR (One definition rule) errors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Since multiple source files are included into one source file,
it can lead to ODR errors. This section contains properties
which help fixing these errors.

The source files marked by :prop_sf:`GENERATED` will be skipped
from unity build. This applies also for the source files marked
with :prop_sf:`SKIP_UNITY_BUILD_INCLUSION`.

The source files that have :prop_sf:`COMPILE_OPTIONS`,
:prop_sf:`COMPILE_DEFINITIONS`, :prop_sf:`COMPILE_FLAGS`, or
:prop_sf:`INCLUDE_DIRECTORIES` will also be skipped.

With the :prop_tgt:`UNITY_BUILD_CODE_BEFORE_INCLUDE` and
:prop_tgt:`UNITY_BUILD_CODE_AFTER_INCLUDE` one can specify code
to be injected in the unity source file before and after every
``#include`` statement.

.. note::

  The order of source files defined in the ``CMakeLists.txt`` will
  be preserved into the generated unity source files. This can
  be used to manually enforce a specific grouping based on the
  :prop_tgt:`UNITY_BUILD_BATCH_SIZE` target property.
