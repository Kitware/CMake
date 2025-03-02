#ifndef __MODULE_H__
#define __MODULE_H__

#if defined(__cplusplus)
#  define INTERNAL 64
#elif !defined(__cplusplus)
#  define INTERNAL 32
#else
#  error "Unable to determine INTERNAL symbol."
#endif

#endif /* __MODULE_H__ */
