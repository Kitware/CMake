#define _CRT_SECURE_NO_WARNINGS // work around 'fopen' deprecation on WIN32

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#endif

#include <string>

#include <stdio.h>

void sleepFor(unsigned seconds)
{
#if defined(_WIN32)
  Sleep(seconds * 1000);
#else
  sleep(seconds);
#endif
}

int main(int argc, char** argv)
{
  // Note: GoogleTest.cmake doesn't actually depend on Google Test as such;
  // it only requires that we produce output in the expected format when
  // invoked with --gtest_list_tests. Thus, we fake that here. This allows us
  // to test the module without actually needing Google Test.
  if (argc > 1 && std::string(argv[1]) == "--gtest_list_tests" &&
      std::string(argv[2]).find("--gtest_output=json:") != std::string::npos) {
    printf("timeout.\n  case\n");
    fflush(stdout);

    std::string output_param(argv[2]);
    std::string::size_type split = output_param.find(":");
    std::string filepath = output_param.substr(split + 1);
    // The full file path is passed
    FILE* ofile = fopen(filepath.c_str(), "wb");

    if (!ofile) {
      fprintf(stderr, "Failed to create file: %s\n", filepath.c_str());
      return 1;
    }
    std::string json_output = "{\n"
                              "      \"tests\": 1,\n"
                              "      \"name\": \"AllTests\",\n"
                              "      \"testsuites\": [\n"
                              "          {\n"
                              "              \"name\": \"timeout\",\n"
                              "              \"tests\": 1,\n"
                              "              \"testsuite\": [\n"
                              "                  { \"name\": \"case\", "
                              "\"file\": \"file.cpp\", \"line\": 42 }\n"
                              "              ]\n"
                              "          }\n"
                              "      ]\n"
                              "  }";
    fprintf(ofile, "%s", json_output.c_str());
    fclose(ofile);

#ifdef discoverySleepSec
    sleepFor(discoverySleepSec);
#endif
    return 0;
  }

#ifdef sleepSec
  sleepFor(sleepSec);
#endif

  return 0;
}
