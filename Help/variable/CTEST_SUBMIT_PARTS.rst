CTEST_SUBMIT_PARTS
------------------

.. versionadded:: 4.4

Specify a :ref:`semicolon-separated list <CMake Language Lists>` of parts
for the :command:`ctest_submit` command to submit in a
:manual:`ctest(1)` :ref:`Dashboard Client` script,
or on the :program:`ctest` command line via the :ctest-dashboard-option:`-D`
option.

Valid part names are the same as those accepted by the ``PARTS`` option of
:command:`ctest_submit`: ``Start``, ``Update``, ``Configure``, ``Build``,
``Test``, ``Coverage``, ``MemCheck``, ``Submit``, ``Notes``, ``ExtraFiles``,
``Upload``, and ``Done``.

If the :command:`ctest_submit` command is given an explicit ``PARTS``
argument, that argument takes precedence and this variable is ignored.

When this variable is empty or unset (and no explicit ``PARTS`` argument is
given), all available parts are submitted (the default behavior).

Example --- submit only the Configure result in a single ``ctest`` invocation::

  ctest -T Configure -T Submit -D CTEST_SUBMIT_PARTS=Configure
