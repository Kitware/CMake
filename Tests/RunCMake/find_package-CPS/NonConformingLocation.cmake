cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

# Bump the diagnostic level up for this specific test. This diagnostic is set
# to ignore by default.
cmake_diagnostic(SET CMD_CPS_NON_CONFORMING_LOCATION WARN)

# Trivial is in PREFIX/cps/ which is no longer a CPS-conforming search
# location.
find_package(Trivial REQUIRED)
