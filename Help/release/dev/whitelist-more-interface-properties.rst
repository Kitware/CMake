whitelist-more-interface-properties
-----------------------------------

* ``INTERFACE`` libraries may now have custom properties set on them if they
  start with either an underscore (``_``) or a lowercase ASCII character. The
  original intention was to only allow properties which made sense for
  ``INTERFACE`` libraries, but it also blocked usage of custom properties.
