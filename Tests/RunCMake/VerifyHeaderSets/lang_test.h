#if defined(__cplusplus) && !defined(EXPECT_CXX)
#  error "__cplusplus defined but EXPECT_CXX not defined"
#endif
#if !defined(__cplusplus) && defined(EXPECT_CXX)
#  error "__cplusplus not defined but EXPECT_CXX defined"
#endif

extern void lang_test_h(void);
