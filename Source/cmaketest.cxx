#include "cmaketest.h"
#include "cmSystemTools.h"

// this is a test driver program for cmake.
main (int argc, char *argv[])
{
  if (argc < 4)
    {
    cerr << "Usage: " << argv[0] << " test-src-dir test-bin-dir test-executable\n";
    return 1;
    }
  
  // does the directory exist ?
  if (!cmSystemTools::FileIsDirectory(argv[2]))
    {
    cmSystemTools::MakeDirectory(argv[2]);
    }

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string output;
  
  // change to the tests directory and run cmake
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(argv[2]);
  std::string ccmd = CMAKE_COMMAND;
  ccmd += " ";
  ccmd += argv[1];
  if (!cmSystemTools::RunCommand(ccmd.c_str(), output))
    {
    cerr << "Error: cmake execution failed\n";
    cerr << output.c_str() << "\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  // now build the test
  if (!cmSystemTools::RunCommand(MAKECOMMAND, output))
    {
    cerr << "Error: " MAKECOMMAND "  execution failed\n";
    cerr << output.c_str() << "\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  // now run the compiled test
  if (!cmSystemTools::RunCommand(argv[3], output))
    {
    cerr << "Error: " << argv[3] << "  execution failed\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }  
  
  // return to the original directory
  cmSystemTools::ChangeDirectory(cwd.c_str());
  return 0;
}
