CMAKE_VERSION
-------------

The CMake version string as up to four non-negative integer components
separated by ``.`` and possibly followed by ``-`` and other information.
The first three components represent the feature level and the fourth
component represents either a bug-fix level or development date.

Release versions and release candidate versions of CMake use the format::

  <major>.<minor>.<patch>[.<tweak>][-rc<n>]

where the ``<tweak>`` component is less than ``20000000``.  Development
versions of CMake use the format::

  <major>.<minor>.<patch>.<date>[-<id>]

where the ``<date>`` component is of format ``CCYYMMDD`` and ``<id>``
may contain arbitrary text.  This represents development as of a
particular date following the ``<major>.<minor>.<patch>`` feature
release.

Individual component values are also available in variables:

* :variable:`CMAKE_MAJOR_VERSION`
* :variable:`CMAKE_MINOR_VERSION`
* :variable:`CMAKE_PATCH_VERSION`
* :variable:`CMAKE_TWEAK_VERSION`

Use the :command:`if` command ``VERSION_LESS``, ``VERSION_EQUAL``, or
``VERSION_GREATER`` operators to compare version string values against
``CMAKE_VERSION`` using a component-wise test.  Version component
values may be 10 or larger so do not attempt to compare version
strings as floating-point numbers.

.. note::

  CMake versions prior to 2.8.2 used three components for the
  feature level and had no bug-fix component.  Release versions
  used an even-valued second component, i.e.
  ``<major>.<even-minor>.<patch>[-rc<n>]``.  Development versions
  used an odd-valued second component with the development date as
  the third component, i.e. ``<major>.<odd-minor>.<date>``.

  The ``CMAKE_VERSION`` variable is defined by CMake 2.6.3 and higher.
  Earlier versions defined only the individual component variables.
