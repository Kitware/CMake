CTEST_SITE
----------

.. versionadded:: 3.1

Specify the CTest ``Site`` setting
in a :manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D` option.

In a dashboard script, set this variable before calling :command:`ctest_start`,
which initializes the ``Site`` setting from it.
