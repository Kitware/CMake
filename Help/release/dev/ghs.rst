ghs
---

* The :generator:`Green Hills MULTI` generator is updated:

  - Added support for architecture selection through
    :variable:`CMAKE_GENERATOR_PLATFORM`:
    e.g. ``arm``, ``ppc``, and ``86``.

  - Added support for toolset selection through
    :variable:`CMAKE_GENERATOR_TOOLSET`,
    e.g. ``comp_201205``, ``comp_201510``, ``comp_201722_beta``.

  - Added support for platform selection through ``GHS_TARGET_PLATFORM``,
    e.g. ``integrity``, ``linux``, ``standalone``, etc.

  - No longer checks that ``arm`` based compilers are installed but ensures
    that the correct ``gbuild.exe`` exists.

  - No longer hard-codes ARM files, BSP, toolset, or OS locations.
