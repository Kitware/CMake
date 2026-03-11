#if defined(__cplusplus) && !defined(EXPECT_CXX)
#  error "__cplusplus defined but EXPECT_CXX not defined"
#endif
#if !defined(__cplusplus) && defined(EXPECT_CXX)
#  error "__cplusplus not defined but EXPECT_CXX defined"
#endif

#if defined(__OBJC__) && !defined(EXPECT_OBJC)
#  error "__OBJC__ defined but EXPECT_OBJC not defined"
#endif
#if !defined(__OBJC__) && defined(EXPECT_OBJC)
#  error "__OBJC__ not defined but EXPECT_OBJC defined"
#endif

extern void lang_test_h(void);
