AIX_SHARED_LIBRARY_SYMLINKS
---------------------------

.. versionadded:: 4.6

On AIX, enable or disable :prop_tgt:`VERSION` and :prop_tgt:`SOVERSION`
file names for a ``SHARED`` library target that is built as a plain shared
object, i.e., when :prop_tgt:`AIX_SHARED_LIBRARY_ARCHIVE` is disabled.
This property has no effect on shared library archives.

* If enabled, the shared object is named ``lib<name>.so.<version>``
  and symlinks ``lib<name>.so.<soversion>`` and ``lib<name>.so``
  are created, as on other UNIX platforms.

  AIX shared objects have no builtin soname, so a dependent records the
  name of the file used to link it.  CMake links dependents through the
  ``lib<name>.so.<soversion>`` symlink so they depend on that name at
  runtime.  If :prop_tgt:`SOVERSION` is not set, dependents link to
  ``lib<name>.so.<version>`` instead.  Projects that do not want
  dependents to record the full version should set :prop_tgt:`SOVERSION`.

* If disabled, a single ``lib<name>.so`` file is produced and the
  :prop_tgt:`VERSION` and :prop_tgt:`SOVERSION` properties are ignored.

This property defaults to :variable:`CMAKE_AIX_SHARED_LIBRARY_SYMLINKS`
if that variable is set when a non-imported ``SHARED`` library target
is created by :command:`add_library`.  If this property is not set,
the default is determined by policy :policy:`CMP0227`.
