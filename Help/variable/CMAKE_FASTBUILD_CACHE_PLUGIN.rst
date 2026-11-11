CMAKE_FASTBUILD_CACHE_PLUGIN
----------------------------

.. versionadded:: 4.6

Specifies a FASTBuild cache plugin in the generated file: the path of a shared
library that FASTBuild loads to store and retrieve cached artifacts in place of
its own cache directory (the ``.CachePluginDLL`` setting).

FASTBuild passes :variable:`CMAKE_FASTBUILD_CACHE_PATH`, or the
``FASTBUILD_CACHE_PATH`` environment variable, and
:variable:`CMAKE_FASTBUILD_CACHE_PLUGIN_CONFIG` to the plugin. As with its own
cache, FASTBuild uses the plugin only in a build with caching enabled: see
:variable:`CMAKE_FASTBUILD_CACHE_MODE`.

See the `FASTBuild caching documentation <https://www.fastbuild.org/docs/features/caching.html>`_
for more information.
