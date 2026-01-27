#include <stdio.h>
#include <string.h>

/* Having this as comment lets gtest_add_tests recognizes the test we fake
   here without requiring googletest
TEST_F( launcher_test, test1 )
{
}
*/

char const gtest_output_json_flag_prefix[] = "--gtest_output=json:";
char const json_output[] = "{\n"
                           "    \"tests\": 1,\n"
                           "    \"name\": \"AllTests\",\n"
                           "    \"testsuites\": [\n"
                           "        {\n"
                           "            \"name\": \"launcher_test\",\n"
                           "            \"tests\": 1,\n"
                           "            \"testsuite\": [\n"
                           "                { \"name\": \"test1\", \"file\": "
                           "\"file.cpp\", \"line\": 42 }\n"
                           "            ]\n"
                           "        }\n"
                           "    ]\n"
                           "}";

int main(int argc, char** argv)
{
  /* Note: Launcher.cmake doesn't actually depend on Google Test as such;
   * it only requires that we produces output in the expected format when
   * invoked with --gtest_list_tests. Thus, we fake that here. This allows us
   * to test the module without actually needing Google Test.  */
  char* filepath;
  FILE* ofile;

  if (argc > 2 && strcmp(argv[1], "--gtest_list_tests") == 0 &&
      strncmp(argv[2], gtest_output_json_flag_prefix,
              strlen(gtest_output_json_flag_prefix)) == 0) {
    printf("launcher_test.\n");
    printf("  test1\n");
    filepath = strchr(argv[2], ':') + 1;
    /* The full file path is passed */
    ofile = fopen(filepath, "wb");

    if (!ofile) {
      fprintf(stderr, "Failed to create file: %s\n", filepath);
      return 1;
    }
    fprintf(ofile, "%s", json_output);
    fclose(ofile);
  }

  printf("launcher_test.test1\n");
  return 0;
}
