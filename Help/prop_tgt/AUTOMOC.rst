AUTOMOC
-------

Should the target be processed with automoc (for Qt projects).

AUTOMOC is a boolean specifying whether CMake will handle the Qt ``moc``
preprocessor automatically, i.e.  without having to use the
:module:`QT4_WRAP_CPP() <FindQt4>` or QT5_WRAP_CPP() macro.  Currently Qt4 and Qt5 are
supported.

When this property is set ``ON``, CMake will scan the
source files at build time and invoke moc accordingly.

* If an ``#include`` statement like ``#include "moc_foo.cpp"`` is found,
  the ``Q_OBJECT`` class declaration is expected in the header, and
  ``moc`` is run on the header file.  A ``moc_foo.cpp`` file will be
  generated from the source's header into the
  ``<CMAKE_CURRENT_BINARY_DIR>/<TARGETNAME>_autogen/include``
  directory which is automatically added to the target's
  :prop_tgt:`INCLUDE_DIRECTORIES`.  This allows the compiler to find the
  included ``moc_foo.cpp`` file regardless of the location the original source.
  However, if multiple source files in different directories do this then their
  generated moc files would collide.  In this case a diagnostic will be issued.

* If an ``#include`` statement like ``#include "foo.moc"`` is found,
  then a ``Q_OBJECT`` is expected in the current source file and ``moc``
  is run on the file itself.  Additionally, header files with the same
  base name (like ``foo.h``) or ``_p`` appended to the base name (like
  ``foo_p.h``) are parsed for ``Q_OBJECT`` macros, and if found, ``moc``
  is also executed on those files.  ``AUTOMOC`` checks multiple header
  alternative extensions, such as ``hpp``, ``hxx`` etc when searching
  for headers.  The resulting moc files, which are not included as shown
  above in any of the source files are included in a generated
  ``moc_compilation.cpp`` file, which is compiled as part of the
  target.

This property is initialized by the value of the :variable:`CMAKE_AUTOMOC`
variable if it is set when a target is created.

Additional command line options for moc can be set via the
:prop_tgt:`AUTOMOC_MOC_OPTIONS` property.

By enabling the :variable:`CMAKE_AUTOMOC_RELAXED_MODE` variable the
rules for searching the files which will be processed by moc can be relaxed.
See the documentation for this variable for more details.

The global property :prop_gbl:`AUTOGEN_TARGETS_FOLDER` can be used to group the
automoc targets together in an IDE, e.g.  in MSVS.

Source C++ files can be excluded from :prop_tgt:`AUTOMOC` processing by
enabling :prop_sf:`SKIP_AUTOMOC` or the broader :prop_sf:`SKIP_AUTOGEN`.

See the :manual:`cmake-qt(7)` manual for more information on using CMake
with Qt.
