USE_FOLDERS
-----------

Use the :prop_tgt:`FOLDER` target property to organize targets into
folders.

.. versionchanged:: 3.26

  CMake treats this property as ``ON`` by default.
  See policy :policy:`CMP0143`.

CMake generators that are capable of organizing into a hierarchy of folders
use the values of the :prop_tgt:`FOLDER` target property to name those
folders. (i.e.: ``Visual Studio`` or ``XCode``)

IDE's can also take advantage of this property to organize CMake targets.
Regardless of generator support.

See also the documentation for the :prop_tgt:`FOLDER` target property.
