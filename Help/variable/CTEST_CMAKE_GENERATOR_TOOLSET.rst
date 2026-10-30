CTEST_CMAKE_GENERATOR_TOOLSET
-----------------------------

Specify the CTest ``CMakeGeneratorToolset`` setting
in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

The :command:`ctest_configure` command passes this value to
:manual:`cmake(1)` with the :cmake-option:`-T` option when
:variable:`CTEST_CONFIGURE_COMMAND` is not set.  See
:variable:`CMAKE_GENERATOR_TOOLSET` for the values supported by each
generator.
