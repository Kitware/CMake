interprocedural_optimization_policy
-----------------------------------

* The :prop_tgt:`INTERPROCEDURAL_OPTIMIZATION` target property is now enforced
  when enabled.  CMake will add IPO flags unconditionally or produce an error
  if it does not know the flags for the current compiler.  The project is now
  responsible to use the :module:`CheckIPOSupported` module to check for IPO
  support before enabling the target property.  See policy :policy:`CMP0069`.
