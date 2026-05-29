# Defines a placeholder-shaped built-in cache variable on the command
# line for a language combination the underlying ``.rst`` manual
# explicitly excludes (``CMAKE_<LANG>_CPPCHECK`` applies only when
# ``<LANG>`` is C or CXX; this test passes ``HIP``).  The matcher is
# intentionally permissive: it accepts the name and returns the
# pattern's Summary, which is installed as the entry's HELPSTRING;
# the per-language restriction is documented in the Summary prose,
# not enforced by the matcher.  This case validates the "permissive
# matching with restriction-in-prose" design end-to-end.
