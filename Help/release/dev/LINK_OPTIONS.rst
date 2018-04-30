LINK_OPTIONS
------------

* CMake gained new capabilities to manage link step:

  * :prop_dir:`LINK_OPTIONS` directory property.
  * :prop_tgt:`LINK_OPTIONS` and :prop_tgt:`INTERFACE_LINK_OPTIONS` target
    properties.
  * :command:`add_link_options` command to add link options in the current
    directory.
  * :command:`target_link_options` command to add link options to targets.
