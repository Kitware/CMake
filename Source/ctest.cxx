#include <stdio.h>
#include "ctest.h"

bool TryExecutable(const char *dir, const char *file,
                   std::string *fullPath, const char *subdir)
{
  // try current directory
  std::string tryPath;
  if (dir && strcmp(dir,""))
    {
    tryPath = dir;
    tryPath += "/";
    }
  
  if (subdir && strcmp(subdir,""))
    {
    tryPath += subdir;
    tryPath += "/";
    }
  
  tryPath += file;
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    *fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    return true;
    }
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    *fullPath = cmSystemTools::CollapseFullPath(tryPath.c_str());
    return true;
    }
  return false;
}


std::string ctest::FindExecutable(const char *exe)
{
  std::string fullPath = "";
  std::string dir;
  std::string file;

  cmSystemTools::SplitProgramPath(exe, dir, file);
  
  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"."))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,""))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"Release"))
    {
    return fullPath;
    }

    if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"Debug"))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"MinSizeRel"))
    {
    return fullPath;
    }

  if (TryExecutable(dir.c_str(),file.c_str(),&fullPath,"RelWithDebInfo"))
    {
    return fullPath;
    }

  // if everything else failed, check the users path
  if (dir != "")
    {
    std::string path = cmSystemTools::FindProgram(file.c_str());
    if (path != "")
      {
      return path;
      }
    }
  
  return fullPath;
}


void ctest::ProcessDirectory(int &passed, int &failed)
{
  // does the DartTestfile.txt exist ?
  if(!cmSystemTools::FileExists("DartTestfile.txt"))
    {
    return;
    }
  
  // parse the file
  std::ifstream fin("DartTestfile.txt");
  if(!fin)
    {
    return;
    }

  std::string name;
  std::vector<std::string> args;
  while ( fin )
    {
    if(cmSystemTools::ParseFunction(fin, name, args))
      {
      if (name == "SUBDIRS")
        {
        std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
        for(std::vector<std::string>::iterator j = args.begin();
            j != args.end(); ++j)
          {   
          std::string nwd = cwd + "/";
          nwd += *j;
          if (cmSystemTools::FileIsDirectory(nwd.c_str()))
            {
            std::cerr << "Changing directory into " << nwd.c_str() << "\n";
            cmSystemTools::ChangeDirectory(nwd.c_str());
            this->ProcessDirectory(passed,failed);
            }
          }
        // return to the original directory
        cmSystemTools::ChangeDirectory(cwd.c_str());
        }

      if (name == "ADD_TEST")
        {
        fprintf(stderr,"Testing %-30s ",args[0].c_str());
        //std::cerr << "Testing " << args[0] << " ... ";
        // find the test executable
        std::string testCommand = this->FindExecutable(args[1].c_str());
        // add the arguments
        std::vector<std::string>::iterator j = args.begin();
        ++j;
        ++j;
        for(;j != args.end(); ++j)
          {   
          testCommand += " ";
          testCommand += *j;
          }
        /**
         * Run an executable command and put the stdout in output.
         */
        std::string output;
        int retVal;
        
        if (!cmSystemTools::RunCommand(testCommand.c_str(), output, 
                                       retVal, false) || retVal != 0)
          {
          std::cerr << " Failed\n";
          if (output != "")
            {
            std::cerr << output.c_str() << "\n";
            }
          failed++;
          }
        else
          {
          std::cerr << " Passed\n";
          if (output != "")
            {
            std::cerr << output.c_str() << "\n";
            }
          passed++;
          }
        }
      }
    }
  
}


// this is a test driver program for cmake.
int main (int argc, char *argv[])
{
  int passed = 0;
  int failed = 0;
  int total;
  
  ctest inst;
  
  // call process directory
  inst.ProcessDirectory(passed, failed);
  total = passed + failed;

  if (total == 0)
    {
    std::cerr << "No tests were found!!!\n";
    }
  else
    {
    float percent = passed * 100.0 / total;
    fprintf(stderr,"%.0f%% tests passed, %i tests failed out of %i\n",
            percent,failed, total);
    }
  
  return failed;
}
