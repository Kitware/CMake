macro (add_hip_test_macro name)
  add_test_macro("${name}" ${ARGN})
  set_property(TEST "${name}" APPEND
    PROPERTY LABELS "HIP")
endmacro ()

add_hip_test_macro(HIP.ArchitectureOff HIPOnlyArchitectureOff)
add_hip_test_macro(HIP.CompileFlags HIPOnlyCompileFlags)
add_hip_test_macro(HIP.EnableStandard HIPEnableStandard)
add_hip_test_macro(HIP.MathFunctions HIPOnlyMathFunctions)
add_hip_test_macro(HIP.TryCompile HIPOnlyTryCompile)
add_hip_test_macro(HIP.WithDefs HIPOnlyWithDefs)
