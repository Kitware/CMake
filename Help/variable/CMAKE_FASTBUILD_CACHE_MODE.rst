CMAKE_FASTBUILD_CACHE_MODE
--------------------------

.. versionadded:: 4.6

Specifies how builds started by :option:`cmake --build` use the FASTBuild
cache, with the values of FASTBuild's ``FASTBUILD_CACHE_MODE`` environment
variable:

``r``
  Retrieve artifacts from the cache (FASTBuild's ``-cacheread``).

``w``
  Store artifacts in the cache (``-cachewrite``).

``rw``
  Both (``-cache``).

If not set, :option:`cmake --build` enables no caching itself, and FASTBuild
uses the cache only as its command line, for example
``cmake --build <dir> -- -cache``, or its ``FASTBUILD_CACHE_MODE``
environment variable says.

:option:`cmake --build` reads this variable from the build tree's cache, so
set it as a cache entry, for example with :cmake-option:`-D` or in a
preset's :ref:`cacheVariables <CMakePresets.configurePresets.cacheVariables>`.
Builds from the generated IDE projects use
:variable:`CMAKE_FASTBUILD_IDE_ARGS` instead.
