VERSION
-------

What version number is this target.

For shared libraries ``VERSION`` and :prop_tgt:`SOVERSION` can be used
to specify the build version and API version respectively.  When building or
installing appropriate symlinks are created if the platform supports
symlinks and the linker supports so-names.  If only one of both is
specified the missing is assumed to have the same version number.  For
executables ``VERSION`` can be used to specify the build version.  When
building or installing appropriate symlinks are created if the
platform supports symlinks.

Windows Versions
^^^^^^^^^^^^^^^^

For shared libraries and executables on Windows the ``VERSION``
attribute is parsed to extract a ``<major>.<minor>`` version number.
These numbers are used as the image version of the binary.

Mach-O Versions
^^^^^^^^^^^^^^^

For shared libraries and executables on Mach-O systems (e.g. macOS, iOS),
the ``VERSION`` property is a fallback to :prop_tgt:`OSX_CURRENT_VERSION`
property which corresponds to *current version* and :prop_tgt:`SOVERSION`
is a fallback to :prop_tgt:`OSX_COMPATIBILITY_VERSION` which corresponds
to *compatiblity version*.  See the :prop_tgt:`FRAMEWORK` target
property for an example.  Versions of Mach-O binaries may be checked with the
``otool -L <binary>`` command.
