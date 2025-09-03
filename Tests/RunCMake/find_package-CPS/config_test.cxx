#ifndef NAME
#  error "NAME is not defined"
#endif
#ifndef MARKER
#  error "MARKER is not defined"
#endif
#ifndef EXPECTED_MARKER
#  error "EXPECTED_MARKER is not defined"
#endif

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#if EXPECTED_MARKER > 0
#  if __cplusplus >= 201103L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201103L)
static_assert(MARKER + 0 == EXPECTED_MARKER,
              "MARKER has unexpected value " STRINGIFY(MARKER));
static_assert(MARKER + 0 == EXPECTED_MARKER, "CONFIG is " STRINGIFY(NAME));
#  else
#    if (MARKER + 0) != (EXPECTED_MARKER)
#      error "MARKER has unexpected value"
#      error "CONFIG mismatch for NAME"
#    endif
#  endif
#endif

#ifdef _WIN32
__declspec(dllexport)
#endif
void dummy_symbol()
{
}
