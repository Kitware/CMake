#ifdef CHECK_SIZE_OF

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif /* HAVE_STDINT_H */

int main()
{
  return sizeof(CHECK_SIZE_OF);
}

#else  /* CHECK_SIZE_OF */

#  error "CHECK_SIZE_OF has to specify the type"

#endif /* CHECK_SIZE_OF */
