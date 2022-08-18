xcode-gpu-validation
--------------------

* The :variable:`CMAKE_XCODE_SCHEME_ENABLE_GPU_API_VALIDATION` variable and
  corresponding :prop_tgt:`XCODE_SCHEME_ENABLE_GPU_API_VALIDATION` target
  property were added to tell the :generator:`Xcode` generator what to put
  in the scheme's ``Metal: API Validation`` setting.

* The :variable:`CMAKE_XCODE_SCHEME_ENABLE_GPU_SHADER_VALIDATION` variable and
  corresponding :prop_tgt:`XCODE_SCHEME_ENABLE_GPU_SHADER_VALIDATION` target
  property were added to tell the :generator:`Xcode` generator what to put
  in the scheme's ``Metal: Shader Validation`` setting.
