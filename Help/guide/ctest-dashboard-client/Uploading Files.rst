.. _`CTest Dashboard Step 6`:

Step 6: Uploading Files
=======================

This step extends :ref:`Step 5 <CTest Dashboard Step 5>` by attaching files
to the build on CDash with the :command:`ctest_upload` command.

.. literalinclude:: Step6/submit.cmake
  :caption: Step6/submit.cmake
  :language: cmake
  :start-at: Bundle the logs

The uploaded files appear on the build's page on CDash, where they can be
downloaded.  This step uploads two files:

``logs.tar.gz``
  An archive of the logs that CTest wrote during this run, created with
  :command:`file(ARCHIVE_CREATE)`.

``cmake.url``
  A file that contains only a URL.  CDash shows it as a link.

Uploads are useful for anything that helps diagnose or reproduce a result
but does not fit in the configure, build, test, or coverage reports, such
as:

* Compiler logs or linker maps.
* Reports from other tools.
* Installation packages.
* Screenshots from GUI tests.  CDash shows images on the build's page.

Requirements
^^^^^^^^^^^^

* The same as :ref:`Step 3 <CTest Dashboard Step 3>`.

Running the Script
^^^^^^^^^^^^^^^^^^

.. code-block:: shell

  ctest -S Step6/submit.cmake -V
