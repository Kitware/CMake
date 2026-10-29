
#ifndef LIBSTATIC_EXPORT_H
#define LIBSTATIC_EXPORT_H

#ifdef LIBSTATIC_STATIC_DEFINE
#  define LIBSTATIC_EXPORT
#  define LIBSTATIC_NO_EXPORT
#else
#  ifndef LIBSTATIC_EXPORT
#    ifdef libstatic_EXPORTS
        /* We are building this library */
#      define LIBSTATIC_EXPORT
#    else
        /* We are using this library */
#      define LIBSTATIC_EXPORT
#    endif
#  endif

#  ifndef LIBSTATIC_NO_EXPORT
#    define LIBSTATIC_NO_EXPORT
#  endif
#endif

#ifndef LIBSTATIC_DEPRECATED
#  define LIBSTATIC_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef LIBSTATIC_DEPRECATED_MSG
#  define LIBSTATIC_DEPRECATED_MSG(msg) __attribute__ ((__deprecated__(msg)))
#endif

#ifndef LIBSTATIC_DEPRECATED_EXPORT
#  define LIBSTATIC_DEPRECATED_EXPORT LIBSTATIC_EXPORT LIBSTATIC_DEPRECATED
#endif

#ifndef LIBSTATIC_DEPRECATED_EXPORT_MSG
#  define LIBSTATIC_DEPRECATED_EXPORT_MSG(msg) LIBSTATIC_EXPORT LIBSTATIC_DEPRECATED_MSG(msg)
#endif

#ifndef LIBSTATIC_DEPRECATED_NO_EXPORT
#  define LIBSTATIC_DEPRECATED_NO_EXPORT LIBSTATIC_NO_EXPORT LIBSTATIC_DEPRECATED
#endif

#ifndef LIBSTATIC_DEPRECATED_NO_EXPORT_MSG
#  define LIBSTATIC_DEPRECATED_NO_EXPORT_MSG(msg) LIBSTATIC_NO_EXPORT LIBSTATIC_DEPRECATED_MSG(msg)
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LIBSTATIC_NO_DEPRECATED
#    define LIBSTATIC_NO_DEPRECATED
#  endif
#endif

#endif /* LIBSTATIC_EXPORT_H */
