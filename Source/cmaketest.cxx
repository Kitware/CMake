#include "cmaketest.h"
#include "cmSystemTools.h"

// this is a test driver program for cmake.
main (int argc, char *argv[])
{
  if (argc < 4)
    {
    std::cerr << "Usage: " << argv[0] << " test-src-dir test-bin-dir test-executable\n";
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
    std::cerr << "Error: cmake execution failed\n";
    std::cerr << output.c_str() << "\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  // now build the test
  std::string makeCommand = MAKEPROGRAM;
  makeCommand += " ";
  makeCommand += argv[3];
#ifdef _WIN32
  makeCommand += ".dsw /MAKE \"ALL_BUILD - Release\" /REBUILD";
#endif  
  if (!cmSystemTools::RunCommand(makeCommand.c_str(), output))
    {
    std::cerr << "Error: " << makeCommand.c_str() <<  "  execution failed\n";
    std::cerr << output.c_str() << "\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  // now run the compiled test if we can find it
  // See if the executable exists as written.
  std::string fullPath;
  if(cmSystemTools::FileExists(argv[3]))
    {
    fullPath = cmSystemTools::CollapseFullPath(argv[3]);
    }
  std::string tryPath = argv[3];
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    }
  
  // try the release extension
  tryPath = "Release/";
  tryPath += cmSystemTools::GetFilenameName(argv[3]);
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    }
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    }

  if (!cmSystemTools::RunCommand(fullPath.c_str(), output))
    {
    std::cerr << "Error: " << fullPath.c_str() << "  execution failed\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }  
  
  // return to the original directory
  cmSystemTools::ChangeDirectory(cwd.c_str());
  return 0;
}
