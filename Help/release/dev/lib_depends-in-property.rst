lib_depends-in-property
-----------------------

* Adds policy CMP0066 which deprecates the ``_LIB_DEPENDS`` variables for
  storing dependency information of targets. Instead, a global propery is
  used to store the information so that it does not persist between CMake
  runs (as it is always regenerated as-needed anyways).
