
# A top-level enable_language() has no ancestor scope to propagate into, so it
# never warns for CMP0220 even when the optional warning is enabled.
set(CMAKE_POLICY_WARNING_CMP0220 ON)

enable_language(CXX)
