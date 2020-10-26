#include <noregen.h>
#include <regen.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
  /* Make sure the noregen header was not regenerated.  */
  if (strcmp("foo", noregen_string) != 0) {
#ifdef XCODE_NEW_BUILD_SYSTEM
    fprintf(stderr,
            "Known limitation: noregen.h was regenerated "
            "but we cannot stop Xcode from doing this!\n");
#else
    printf("FAILED: noregen.h was regenerated!\n");
    return 1;
#endif
  }

  /* Print out the string that should have been regenerated.  */
  printf("%s\n", regen_string);
  fflush(stdout);
  return 0;
}
