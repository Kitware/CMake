#include "cmaketest.h"
#include "cmSystemTools.h"
#include "cmake.h"
#include "cmListFileCache.h"
#include "cmMakefileGenerator.h"
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__BORLANDC__)
#include "windows.h"
#endif

// this is a test driver program for cmake.
int main (int argc, char *argv[])
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
  const char* sourceDirectory = argv[1];
  const char* binaryDirectory = argv[2];
  const char* executableName = argv[3];
  const char* executableDirectory = "";
  if(argc > 4)
    {
    executableDirectory = argv[4];
    }
  
  /**
   * Run an executable command and put the stdout in output.
   */
  std::string output;
  
  // change to the tests directory and run cmake
  // use the cmake object instead of calling cmake
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(binaryDirectory);
  cmake cm;
  std::vector<std::string> args;
  // make sure the same generator is used
  // use this program as the cmake to be run, it should not
  // be run that way but the cmake object requires a vailid path
  std::string cmakeCommand = CMAKE_COMMAND;
  if(cmakeCommand[0] == '\\' && cmakeCommand[1] == '\"')
    {
    cmakeCommand = cmakeCommand.substr(2, cmakeCommand.size()-4);
    }
  if(cmakeCommand[0] == '\"')
    {
    cmakeCommand = cmakeCommand.substr(1, cmakeCommand.size()-2);
    }
  args.push_back(cmakeCommand.c_str());
  args.push_back(sourceDirectory);
  std::string generator = "-G";
  generator += CMAKE_GENERATOR;
  args.push_back(generator);

  if (cm.Generate(args) != 0)
    {
    std::cerr << "Error: cmake execution failed\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cmListFileCache::GetInstance()->ClearCache();
  // now build the test
  std::string makeCommand = MAKEPROGRAM;
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);
  // if msdev is the make program then do the following
  if(lowerCaseCommand.find("msdev") != std::string::npos)
    {
    // if there are spaces in the makeCommand, assume a full path
    // and convert it to a path with no spaces in it as the
    // RunCommand does not like spaces
    if(makeCommand.find(' ') != std::string::npos)
      {
      char *buffer = new char[makeCommand.size()+1];
#if defined(_WIN32) && !defined(__CYGWIN__)      
      if(GetShortPathName(makeCommand.c_str(), buffer, 
                          makeCommand.size()+1) != 0)
        {
        makeCommand = buffer;
        }
#endif
      delete [] buffer;\
      }
    makeCommand += " ";
    makeCommand += executableName;
    makeCommand += ".dsw /MAKE \"ALL_BUILD - Debug\" /REBUILD";
    }
  else
    {
    // assume a make sytle program
    makeCommand += " all";
    }
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
  if(cmSystemTools::FileExists(executableName))
    {
    fullPath = cmSystemTools::CollapseFullPath(executableName);
    }
  std::string tryPath = executableName;
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    }
  // try the Debug extension
  tryPath = "Debug/";
  tryPath += cmSystemTools::GetFilenameName(executableName);
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    }
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    }
  tryPath = executableDirectory;
  tryPath += "/";
  tryPath += executableName;
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    }
  tryPath = executableDirectory;
  tryPath += "/Debug/";
  tryPath += executableName;
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    }
  if(!cmSystemTools::FileExists(fullPath.c_str()))
    {
    std::cerr << "Could not find path to executable, perhaps it was not built: " <<
      executableName << "\n";
    std::cerr << "Error: " << fullPath.c_str() << "  execution failed\n";
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
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
  cmMakefileGenerator::UnRegisterGenerators();
  return 0;
}
