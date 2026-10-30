CTEST_CMAKE_GENERATOR
---------------------

Specify the CTest ``CMakeGenerator`` setting
in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

The :command:`ctest_configure` command passes this generator to
:manual:`cmake(1)` when :variable:`CTEST_CONFIGURE_COMMAND` is not set.
The :command:`ctest_build` command also uses it to compute the build
command line when :variable:`CTEST_BUILD_COMMAND` is not set.

See also :variable:`CTEST_CMAKE_GENERATOR_PLATFORM` and
:variable:`CTEST_CMAKE_GENERATOR_TOOLSET`.
