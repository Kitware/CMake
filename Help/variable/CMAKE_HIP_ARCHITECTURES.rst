CMAKE_HIP_ARCHITECTURES
-----------------------

.. versionadded:: 3.21

Default value for :prop_tgt:`HIP_ARCHITECTURES` property of targets.

This is initialized to the architectures reported by ``rocm_agent_enumerator``,
if available, and otherwise to the default chosen by the compiler.

This variable is used to initialize the :prop_tgt:`HIP_ARCHITECTURES` property
on all targets. See the target property for additional information.
