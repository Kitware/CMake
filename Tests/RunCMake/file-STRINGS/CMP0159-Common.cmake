function (output_results msg)
  message(STATUS "results from: ${msg}")
  message(STATUS "CMAKE_MATCH_0: -->${CMAKE_MATCH_0}<--")
  message(STATUS "CMAKE_MATCH_1: -->${CMAKE_MATCH_1}<--")
  message(STATUS "CMAKE_MATCH_2: -->${CMAKE_MATCH_2}<--")
  message(STATUS "CMAKE_MATCH_COUNT: -->${CMAKE_MATCH_COUNT}<--")
endfunction ()

# Populate `CMAKE_MATCH_<n>` with some initial value
string(REGEX MATCH "(.*):" _ "Initial-value:")
file(STRINGS CMP0159.txt _ REGEX "(.*): (.*)")
output_results(CMP0159)
