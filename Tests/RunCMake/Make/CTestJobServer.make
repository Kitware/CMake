NoPipe:
	env MAKEFLAGS= $(CMAKE_CTEST_COMMAND) -j6
.PHONY: NoPipe

NoTests:
	+$(CMAKE_CTEST_COMMAND) -j6 -R NoTests
.PHONY: NoTests

Tests:
	+$(CMAKE_CTEST_COMMAND) -j6
.PHONY: Tests
