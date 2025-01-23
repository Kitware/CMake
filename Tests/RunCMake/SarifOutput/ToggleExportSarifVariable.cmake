# Generate potential SARIF results
include("${CMAKE_CURRENT_LIST_DIR}/GenerateSarifResults.cmake")

# Enable SARIF logging at the end for the most behavior coverage
# All results should be captured regardless of when enabled
set(CMAKE_EXPORT_SARIF ON CACHE BOOL "Export SARIF results" FORCE)
