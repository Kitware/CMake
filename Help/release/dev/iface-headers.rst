iface-headers
-------------

* ``INTERFACE`` library can now have :prop_tgt:`PUBLIC_HEADER` and
  :prop_tgt:`PRIVATE_HEADER` properties set. The headers specified by those
  properties can be installed using the :command:`install(TARGETS)` command by
  passing the ``PUBLIC_HEADER`` and ``PRIVATE_HEADER`` arguments respectively.
