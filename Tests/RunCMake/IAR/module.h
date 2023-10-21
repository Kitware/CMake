#ifndef __MODULE_H__
#define __MODULE_H__

#if defined(__cplusplus)
#  define INTERNAL 64
#elif !defined(__cplusplus) && defined(__IAR_SYSTEMS_ICC__)
#  define INTERNAL 32
#elif defined(__IAR_SYSTEMS_ASM__)
#  define INTERNAL 16
#else
#  error "Unable to determine INTERNAL symbol."
#endif /* __IAR_SYSTEMS_ICC */

#endif /* __MODULE_H__ */
