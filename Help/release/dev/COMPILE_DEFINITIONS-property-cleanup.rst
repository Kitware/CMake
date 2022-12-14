COMPILE_DEFINITIONS-property-cleanup
------------------------------------

* For all ``COMPILE_DEFINITIONS`` properties, any leading ``-D`` on an item
  will be removed regardless how to was defined: as is or inside a generator
  expression.
