/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmLoadCommandCommand.h"
#include "cmCPluginAPI.h"
#include "cmCPluginAPI.cxx"
#include "cmDynamicLoader.h"

// a class for loadabple commands
class cmLoadedCommand : public cmCommand
{
public:
  cmLoadedCommand() {
    memset(&this->info,0,sizeof(this->info)); 
    this->info.CAPI = &cmStaticCAPI;
  }
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
      cmLoadedCommand *newC = new cmLoadedCommand;
      // we must copy when we clone
      newC->m_commandName = this->m_commandName;
      memcpy(&newC->info,&this->info,sizeof(info));
      return newC;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {
    return (info.m_Inherited != 0 ? true : false);
  }
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {
    return m_commandName.c_str();
  }
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
      if (this->info.GetTerseDocumentation)
        {
        return info.GetTerseDocumentation();
        }
      else
        {
        return "LoadedCommand without any additional documentation";
        }
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      if (this->info.GetFullDocumentation)
        {
        return info.GetFullDocumentation();
        }
      else
        {
        return "LoadedCommand without any additional documentation";
        }
    }
  
  cmTypeMacro(cmLoadedCommand, cmCommand);

  cmLoadedCommandInfo info;
  std::string m_commandName;
};

bool cmLoadedCommand::InitialPass(std::vector<std::string> const& args)
{
  if (!info.InitialPass)
    {
    return true;
    }
  
  // create argc and argv and then invoke the command
  int argc = static_cast<int> (args.size());
  char **argv = NULL;
  if (argc)
    {
    argv = (char **)malloc(argc*sizeof(char *));
    }
  int i;
  for (i = 0; i < argc; ++i)
    {
    argv[i] = strdup(args[i].c_str());
    }
  int result = info.InitialPass((void *)&info,(void *)this->m_Makefile,argc,argv);
  cmFreeArguments(argc,argv);
  if (result)
    {
    return true;
    }
  return false;
}

void cmLoadedCommand::FinalPass()
{
  if (this->info.FinalPass)
    {
    this->info.FinalPass((void *)&this->info,(void *)this->m_Makefile);
    }
  
}

// cmLoadCommandCommand
bool cmLoadCommandCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 1 )
    {
    return true;
    }
  
  // the file must exist
  std::string fullPath = cmDynamicLoader::LibPrefix();
  fullPath += argsIn[0] + cmDynamicLoader::LibExtension();

  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  // search for the file
  std::vector<std::string> path;
  for (unsigned int j = 1; j < args.size(); j++)
    {
    // expand variables
    std::string exp = args[j];
    cmSystemTools::ExpandRegistryValues(exp);
    
    // Glob the entry in case of wildcards.
    cmSystemTools::GlobDirs(exp.c_str(), path);
    }

  // Try to find the program.
  fullPath = cmSystemTools::FindFile(fullPath.c_str(), path);
  if (fullPath == "")
    {
    this->SetError("Attempt to load command failed.");
    return false;
    }
  
  // try loading the shared library / dll
  cmLibHandle lib = cmDynamicLoader::OpenLibrary(fullPath.c_str());
  if(lib)
    {
    // Look for the symbol cmLoad, cmGetFactoryCompilerUsed,
    // and cmGetFactoryVersion in the library
    CM_NAME_FUNCTION nameFunction
      = (CM_NAME_FUNCTION)
      cmDynamicLoader::GetSymbolAddress(lib, "cmGetName");
    CM_INIT_FUNCTION initFunction
      = (CM_INIT_FUNCTION)
      cmDynamicLoader::GetSymbolAddress(lib, "cmInitializeCommand");
    // if the symbol is found call it to set the name on the 
    // function blocker
    if(nameFunction)
      {
      // create a function blocker and set it up
      cmLoadedCommand *f = new cmLoadedCommand();
      f->m_commandName = (*nameFunction)();
      (*initFunction)(&f->info);
      m_Makefile->AddCommand(f);
      }
    }
  return true;
}

