#if defined(CXXFEATURES_CSTDINT_FOUND)
#include <cstdint>
#endif

#include <sys/types.h>

struct thing {
    unsigned char one;
#if defined(CXXFEATURES_CSTDINT_FOUND)
    uint32_t four;
#endif
#if defined(CXXFEATURES_LONG_LONG_FOUND)
    long long eight;
#endif
};

#include <stdio.h>

int main()
{
#if defined (CXXFEATURES_NULLPTR_FOUND)
    nullptr_t *nix = nullptr;
#else /* CXXFEATURES_NULLPTR_FOUND */
    void *nix = 0;
#endif /* CXXFEATURES_NULLPTR_FOUND */

#if defined(CXXFEATURES_STATIC_ASSERT_FOUND)
    static_assert(1 < 42, "Your C++ compiler is b0rked");
#endif /* CXXFEATURES_STATIC_ASSERT_FOUND */

#if defined(CXXFEATURES___FUNC___FOUND)
    const char *funcname = __func__;
    printf("the name of main() function is: %s\n", funcname);
#endif /* CXXFEATURES_FUNC_FOUND */

#if defined(CXXFEATURES_SIZEOF_MEMBER_FOUND)
    size_t onesize = sizeof(thing::one);
#if defined(CXXFEATURES_STATIC_ASSERT_FOUND)
    static_assert(sizeof(thing::one) == 1, "Your char is not one byte long");
#endif /* CXXFEATURES_STATIC_ASSERT_FOUND */

#if defined(CXXFEATURES_CSTDINT_FOUND)
    size_t foursize = sizeof(thing::four);
#if defined(CXXFEATURES_STATIC_ASSERT_FOUND)
    static_assert(sizeof(thing::four) == 4, "Your uint32_t is not 32 bit long");
#endif /* CXXFEATURES_STATIC_ASSERT_FOUND */
#endif /* CXXFEATURES_CSTDINT_FOUND */
#if defined(CXXFEATURES_LONG_LONG_FOUND)
    size_t eightsize = sizeof(thing::eight);
#if defined(CXXFEATURES_STATIC_ASSERT_FOUND)
    static_assert(sizeof(thing::eight) == 8, "Your long long is not 64 bit long");
#endif /* CXXFEATURES_STATIC_ASSERT_FOUND */
#endif /* CXXFEATURES_LONG_LONG_FOUND */
#endif /* CXXFEATURES_SIZEOF_MEMBER_FOUND */

    return !!nix;
}
