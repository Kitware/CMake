WCDH_allow_unsupported
----------------------

* The :module:`WriteCompilerDetectionHeader` module gained the
  ``ALLOW_UNKNOWN_COMPILERS`` and ``ALLOW_UNKNOWN_COMPILER_VERSIONS`` options
  that allow creation of headers that will work also with unknown or old
  compilers by simply assuming they do not support any of the requested
  features.
