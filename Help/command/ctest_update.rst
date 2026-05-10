ctest_update
------------

Perform the :ref:`CTest Update Step` as a :ref:`Dashboard Client`.

.. code-block:: cmake

  ctest_update([SOURCE <source-dir>]
               [RETURN_VALUE <result-var>]
               [VERSION_ONLY]
               [VERSION_OVERRIDE <version>]
               [CAPTURE_CMAKE_ERROR <result-var>]
               [QUIET])

Update the source tree from version control and record results in
``Update.xml`` for submission with the :command:`ctest_submit` command.

The options are:

``SOURCE <source-dir>``
  Specify the source directory.  If not given, the
  :variable:`CTEST_SOURCE_DIRECTORY` variable is used.

``RETURN_VALUE <result-var>``
  Store in the ``<result-var>`` variable the number of files
  updated or ``-1`` on error.

``VERSION_ONLY``
  .. versionadded:: 4.4

  Record the currently checked out revision without updating the source tree to
  a different version.  This is equivalent to setting the
  :variable:`CTEST_UPDATE_VERSION_ONLY` variable.

``VERSION_OVERRIDE <version>``
  .. versionadded:: 4.4

  Report ``<version>`` as the current revision without querying or updating the
  source tree.  This is equivalent to setting the
  :variable:`CTEST_UPDATE_VERSION_OVERRIDE` variable, and supersedes
  ``VERSION_ONLY``.

``CAPTURE_CMAKE_ERROR <result-var>``
  .. versionadded:: 3.13

  Store in the ``<result-var>`` variable ``-1`` if there are any errors running
  the command and prevent :manual:`ctest(1)` from returning non-zero if an
  error occurs.

``QUIET``
  .. versionadded:: 3.3

  Suppress any CTest-specific non-error messages that would have otherwise
  been printed to the console.  CTest will still report
  the new revision of the repository and any conflicting files
  that were found.

By default, the update follows the version control branch currently checked
out in the source directory.  The ``VERSION_ONLY`` and
``VERSION_OVERRIDE`` options instead record revision information without
changing the source tree.  See the :ref:`CTest Update Step` documentation
for information about further variables that change the behavior of
``ctest_update()``.
