# Recipe that wants interactive terminal access: prefixed with the toggle.
add_custom_target(term ALL
  COMMAND $(CMAKE_COMMAND) -E true
  USES_TERMINAL)

# Ordinary recipe: no prefix at all.
add_custom_target(plain ALL
  COMMAND $(CMAKE_COMMAND) -E true)

# Jobserver-aware recipe: keeps the literal '+' prefix.
add_custom_target(jsa ALL
  COMMAND $(CMAKE_COMMAND) -E true
  JOB_SERVER_AWARE ON)

# Both jobserver-aware and interactive: the '+' branch wins, so the toggle
# prefix must not be added (no doubled prefix).
add_custom_target(both ALL
  COMMAND $(CMAKE_COMMAND) -E true
  USES_TERMINAL
  JOB_SERVER_AWARE ON)
