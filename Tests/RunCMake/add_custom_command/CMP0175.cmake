enable_language(CXX)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/main.cpp "int main() {}")
add_executable(main ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)

#============================================================================
# add_custom_command(TARGET)
#============================================================================

# Unsupported keywords. Need to test them in batches to avoid other checks.
add_custom_command(TARGET main
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E true

  # None of the following are allowed for the TARGET form

  #APPEND   # Has its own check requiring OUTPUT to be set
  #CODEGEN  # Other checks will fail before the CMP0175 check
  DEPENDS valueDoesNotMatterHere
  DEPENDS_EXPLICIT_ONLY YES
  DEPFILE valueDoesNotMatterHere
  #IMPLICIT_DEPENDS  # Earlier check fails when DEPFILE is present
  JOB_POOL valueDoesNotMatterHere
  MAIN_DEPENDENCY valueDoesNotMatterHere
  #OUTPUT   # Other checks will fail before the CMP0175 check
  #OUTPUTS  # Special case, not a documented keyword (used for deprecated form)
  #SOURCE   # Old signature, special handling makes it hard to check
  #USES_TERMINAL
)
add_custom_command(TARGET main
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E true
  # Has to be tested separately due to separate check for clash with DEPFILE
  IMPLICIT_DEPENDS valueDoesNotMatterHere
  # Has to be tested separately due to separate check for clash with JOB_POOL
  USES_TERMINAL NO
)

# Missing any PRE_BUILD, PRE_LINK, or POST_BUILD
add_custom_command(TARGET main
  COMMAND ${CMAKE_COMMAND} -E true
)

# More than one of PRE_BUILD, PRE_LINK, or POST_BUILD
add_custom_command(TARGET main
  PRE_BUILD PRE_LINK POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E true
)

# Missing COMMAND
add_custom_command(TARGET main
  POST_BUILD
  COMMENT "Need at least 4 arguments, so added this comment"
)

#============================================================================
# add_custom_command(OUTPUT)
#============================================================================

add_custom_command(OUTPUT blah.txt
  OUTPUTS
  POST_BUILD
  PRE_BUILD
  PRE_LINK
  SOURCE
)
