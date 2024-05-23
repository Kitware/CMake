Swift_MODULE_DIRECTORY
----------------------

.. versionadded:: 3.15

Specify output directory for Swift modules provided by the target.

If the target contains Swift source files, this specifies the directory in which
the modules will be placed.  When this property is not set, the modules will be
placed in the build directory corresponding to the target's source directory.
If the variable :variable:`CMAKE_Swift_MODULE_DIRECTORY` is set when a target is
created its value is used to initialize this property.

.. warning::

  This property does not currently provide a way to express per-config
  module directories, so use with multi-config generators is problematic:

  * The :generator:`Xcode` generator does not implement the property at all.

  * The :generator:`Ninja Multi-Config` generator implements this property,
    but module files generated for different build configurations have the
    same path, which can lead to subtle problems when building more than
    one configuration.
