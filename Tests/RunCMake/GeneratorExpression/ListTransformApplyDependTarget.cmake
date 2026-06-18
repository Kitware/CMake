enable_language(C)

# `provider` is EXCLUDE_FROM_ALL, so it builds only when another target declares
# a build-order dependency on it.
add_library(provider STATIC ListTransformApplyDependTarget.c)
set_property(TARGET provider PROPERTY EXCLUDE_FROM_ALL 1)

# `consumer`'s only reference to `provider` is the $<TARGET_FILE:$<_0>> in the
# APPLY body.  CMake derives the build-order dependency from that reference only
# if APPLY propagates the target back out (DependTargets).  If that regresses,
# `provider` is never built and the copy below fails for lack of the archive, so
# a green build of `consumer` alone is the regression guard.
add_custom_target(consumer
  COMMAND "${CMAKE_COMMAND}" -E copy
    "$<LIST:TRANSFORM,provider,APPLY,$<TARGET_FILE:$<_0>>>"
    "${CMAKE_CURRENT_BINARY_DIR}/copied.out"
  VERBATIM
)
