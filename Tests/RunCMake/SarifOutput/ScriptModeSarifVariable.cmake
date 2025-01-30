# Try enabling SARIF output in script mode
# No file should be generated since script mode ignores the variable
set(CMAKE_EXPORT_SARIF ON CACHE BOOL "Export SARIF results" FORCE)
