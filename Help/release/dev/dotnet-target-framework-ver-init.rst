dotnet-target-fw-initialization
-------------------------------

* The :prop_tgt:`DOTNET_TARGET_FRAMEWORK_VERSION` target property
  was introduced as replacement for
  :prop_tgt:`VS_DOTNET_TARGET_FRAMEWORK_VERSION`, which is considered
  deprecated now.

* The :variable:`CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION` variable
  was defined to initialize all
  :prop_tgt:`DOTNET_TARGET_FRAMEWORK_VERSION` target properties.
