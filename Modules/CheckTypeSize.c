#ifdef CHECK_TYPE_SIZE_TYPE

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif /* HAVE_STDINT_H */

int main()
{
  return sizeof(CHECK_TYPE_SIZE_TYPE);
}

#else  /* CHECK_TYPE_SIZE_TYPE */

#  error "CHECK_TYPE_SIZE_TYPE has to specify the type"

#endif /* CHECK_TYPE_SIZE_TYPE */
