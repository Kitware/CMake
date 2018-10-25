qt_wrap_ui
----------

Create Qt user interfaces Wrappers.

.. code-block:: cmake

  qt_wrap_ui(resultingLibraryName HeadersDestName
             SourcesDestName SourceLists ...)

Produces .h and .cxx files for all the .ui files listed in the
``SourceLists``.  The .h files will be added to the library using the
``HeadersDestNamesource`` list.  The .cxx files will be added to the
library using the ``SourcesDestNamesource`` list.
