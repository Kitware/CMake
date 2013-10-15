ctest_update
------------

Update the work tree from version control.

::

  ctest_update([SOURCE source] [RETURN_VALUE res])

Updates the given source directory and stores results in Update.xml.
If no SOURCE is given, the CTEST_SOURCE_DIRECTORY variable is used.
The RETURN_VALUE option specifies a variable in which to store the
result, which is the number of files updated or -1 on error.
