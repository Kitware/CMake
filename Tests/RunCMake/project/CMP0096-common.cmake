include(PrintVersions.cmake)

# Test leading zeros motivating this policy.
project(DateVersion VERSION 2019.07.06 LANGUAGES NONE)
print_versions(DateVersion)

# Overflow version component in OLD behavior.
project(LongVersion VERSION 4294967297 #[[ uint32_max + 2 ]] LANGUAGES NONE)
print_versions(LongVersion)
