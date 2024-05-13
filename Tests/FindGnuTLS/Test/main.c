#include <assert.h>
#include <gnutls/gnutls.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  // test the linker
  gnutls_session_t session;
  if (gnutls_init(&session, GNUTLS_CLIENT)) {
    gnutls_deinit(session);
  }

  // check the version
  char gnutls_version_string[16];
  snprintf(gnutls_version_string, 16, "%i.%i.%i", GNUTLS_VERSION_MAJOR,
           GNUTLS_VERSION_MINOR, GNUTLS_VERSION_PATCH);
  assert(strcmp(gnutls_version_string, CMAKE_EXPECTED_GNUTLS_VERSION) == 0);

  return 0;
}
