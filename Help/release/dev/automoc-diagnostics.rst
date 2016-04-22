automoc-diagnostics
-------------------

* :prop_tgt:`AUTOMOC` now diagnoses name collisions when multiple source
  files in different directories use ``#include <moc_foo.cpp>`` with the
  same name (because the generated ``moc_foo.cpp`` files would collide).
