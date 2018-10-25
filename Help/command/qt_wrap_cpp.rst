qt_wrap_cpp
-----------

Create Qt Wrappers.

.. code-block:: cmake

  qt_wrap_cpp(resultingLibraryName DestName SourceLists ...)

Produces moc files for all the .h files listed in the SourceLists.  The
moc files will be added to the library using the ``DestName`` source list.
