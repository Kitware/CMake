
#include <iostream>
#include <fstream>
#include <cstring>

int main(int argc, char** argv, char** envp)
{
  char** env;

  for(env = envp; *env != 0; env++) {
    char* thisEnv = *env;

    // Check if the environment variable got set
    if(0 == strcmp(thisEnv,"SOME_VAR_FOR_TESTING=True")) {

      std::cout<<"CMake custom command found the environment variable!"<<std::endl;

      // Write out the build product
      std::ofstream result_file;
      result_file.open("env_result.txt");
      result_file << thisEnv;
      result_file.close();

      // Success!
      return 0;
    }
  }

  // Not success!
  std::cerr<<"CMake custom command could not find the environment variable!"<<std::endl;

  return -1;
}

