allow-OBJECT-library-extra-sources
----------------------------------

* :ref:`Object Libraries` may now have extra sources that do not
  compile to object files so long as they would not affect linking
  of a normal library (e.g. ``.dat`` is okay but not ``.def``).
