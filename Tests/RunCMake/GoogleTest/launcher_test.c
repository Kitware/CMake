#include <stdio.h>
#include <string.h>

/* Having this as comment lets gtest_add_tests recognizes the test we fake
   here without requiring googletest
TEST_F( launcher_test, test1 )
{
}
*/

int main(int argc, char** argv)
{
  /* Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
   * it only requires that we produces output in the expected format when
   * invoked with --gtest_list_tests. Thus, we fake that here. This allows us
   * to test the module without actually needing Google Test.  */
  if (argc > 1 && strcmp(argv[1], "--gtest_list_tests") == 0) {
    printf("launcher_test.\n");
    printf("  test1\n");
  }

  printf("launcher_test.test1\n");
  return 0;
}
