NoPipe:
	env MAKEFLAGS= $(CMAKE_CTEST_COMMAND) -j0
.PHONY: NoPipe

NoTests:
	+$(CMAKE_CTEST_COMMAND) -j -R NoTests
.PHONY: NoTests

Tests:
	+$(CMAKE_CTEST_COMMAND) -j
.PHONY: Tests
