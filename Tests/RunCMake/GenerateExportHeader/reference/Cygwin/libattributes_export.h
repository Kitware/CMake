
#ifndef LIBATTRIBUTES_EXPORT_H
#define LIBATTRIBUTES_EXPORT_H

#ifdef LIBATTRIBUTES_STATIC_DEFINE
#  define LIBATTRIBUTES_EXPORT
#  define LIBATTRIBUTES_NO_EXPORT
#else
#  ifndef LIBATTRIBUTES_EXPORT
#    ifdef libattributes_EXPORTS
        /* We are building this library */
#      if defined(__cplusplus) && __cplusplus >= 201103L && defined(__has_cpp_attribute) && !defined(Q_MOC_RUN)
#        if __has_cpp_attribute(gnu::dllexport)
#          define LIBATTRIBUTES_EXPORT [[gnu::dllexport]]
#        endif
#      endif
#      ifndef LIBATTRIBUTES_EXPORT
#        define LIBATTRIBUTES_EXPORT __declspec(dllexport)
#      endif
#    else
        /* We are using this library */
#      if defined(__cplusplus) && __cplusplus >= 201103L && defined(__has_cpp_attribute) && !defined(Q_MOC_RUN)
#        if __has_cpp_attribute(gnu::dllimport)
#          define LIBATTRIBUTES_EXPORT [[gnu::dllimport]]
#        endif
#      endif
#      ifndef LIBATTRIBUTES_EXPORT
#        define LIBATTRIBUTES_EXPORT __declspec(dllimport)
#      endif
#    endif
#  endif

#  ifndef LIBATTRIBUTES_NO_EXPORT
#    define LIBATTRIBUTES_NO_EXPORT
#  endif
#endif

#ifndef LIBATTRIBUTES_DEPRECATED
#  if defined(__cplusplus) && __cplusplus >= 201103L && defined(__has_cpp_attribute) && !defined(Q_MOC_RUN)
#    if __has_cpp_attribute(gnu::deprecated)
#      define LIBATTRIBUTES_DEPRECATED [[gnu::deprecated]]
#    endif
#  endif
#  ifndef LIBATTRIBUTES_DEPRECATED
#    define LIBATTRIBUTES_DEPRECATED __attribute__ ((__deprecated__))
#  endif
#endif

#ifndef LIBATTRIBUTES_DEPRECATED_EXPORT
#  define LIBATTRIBUTES_DEPRECATED_EXPORT LIBATTRIBUTES_EXPORT LIBATTRIBUTES_DEPRECATED
#endif

#ifndef LIBATTRIBUTES_DEPRECATED_NO_EXPORT
#  define LIBATTRIBUTES_DEPRECATED_NO_EXPORT LIBATTRIBUTES_NO_EXPORT LIBATTRIBUTES_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LIBATTRIBUTES_NO_DEPRECATED
#    define LIBATTRIBUTES_NO_DEPRECATED
#  endif
#endif

#endif /* LIBATTRIBUTES_EXPORT_H */
