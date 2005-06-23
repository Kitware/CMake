/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMacroCommand.h"

#include "cmake.h"

// define the class for macro commands
class cmMacroHelperCommand : public cmCommand
{
public:
  cmMacroHelperCommand() {}
  
  ///! clean up any memory allocated by the macro
  ~cmMacroHelperCommand() {};
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
  {
    cmMacroHelperCommand *newC = new cmMacroHelperCommand;
    // we must copy when we clone
    newC->m_Args = this->m_Args;
    newC->m_Functions = this->m_Functions;
    return newC;
  }
  
  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InvokeInitialPass(const std::vector<cmListFileArgument>& args);

  virtual bool InitialPass(std::vector<std::string> const&) { return false; };
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return this->m_Args[0].c_str(); }
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
  {
    std::string docs = "Macro named: ";
    docs += this->GetName();
    return docs.c_str();
  }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
  {
    return this->GetTerseDocumentation();
  }
  
  cmTypeMacro(cmMacroHelperCommand, cmCommand);

  std::vector<std::string> m_Args;
  std::vector<cmListFileFunction> m_Functions;
};


bool cmMacroHelperCommand::InvokeInitialPass
(const std::vector<cmListFileArgument>& args)
{
  // Expand the argument list to the macro.
  std::vector<std::string> expandedArgs;
  m_Makefile->ExpandArguments(args, expandedArgs);
  
  std::string tmps;
  cmListFileArgument arg;
  std::string variable;

  // make sure the number of arguments passed is at least the number
  // required by the signature
  if (expandedArgs.size() < m_Args.size() - 1)
    {
    std::string errorMsg = 
      "Macro invoked with incorrect arguments for macro named: ";
    errorMsg += m_Args[0];
    this->SetError(errorMsg.c_str());
    return false;
    }
  
  // set the value of argc
  cmOStringStream argcDefStream;
  argcDefStream << expandedArgs.size();
  std::string argcDef = argcDefStream.str();
  
  // declare varuiables for ARGV ARGN but do not compute until needed
  std::string argvDef;
  std::string argnDef;
  bool argvDefInitialized = false;

  // save the current definitions of all vars that we will be setting
  std::string oldARGC;
  if (m_Makefile->GetDefinition("ARGC"))
    {
    oldARGC = m_Makefile->GetDefinition("ARGC");
    }
  m_Makefile->AddDefinition("ARGC",argcDef.c_str());

  // store ARGN, ARGV
  std::vector<std::string> oldARGVArgs;
  std::vector<std::string>::const_iterator eit;
  std::vector<std::string>::size_type cnt = 0;
  char argvName[60];
  for ( eit = expandedArgs.begin(); eit != expandedArgs.end(); ++eit )
    {
    if ( cnt >= m_Args.size()-1 )
      {
      if ( argnDef.size() > 0 )
        {
        argnDef += ";";
        }
      argnDef += *eit;
      }
    if ( argvDef.size() > 0 )
      {
      argvDef += ";";
      }
    argvDef += *eit;
    oldARGVArgs.push_back(std::string());
    sprintf(argvName,"ARGV%i",cnt);
    if (m_Makefile->GetDefinition(argvName))
      {
      oldARGVArgs[cnt] = m_Makefile->GetDefinition(argvName);
      }
    m_Makefile->AddDefinition(argvName,eit->c_str());
    cnt++;
    }
  std::string oldARGN;
  if (m_Makefile->GetDefinition("ARGN"))
    {
    oldARGN = m_Makefile->GetDefinition("ARGN");
    }
  m_Makefile->AddDefinition("ARGN",argnDef.c_str());
  std::string oldARGV;
  if (m_Makefile->GetDefinition("ARGV"))
    {
    oldARGV = m_Makefile->GetDefinition("ARGV");
    }
  m_Makefile->AddDefinition("ARGV",argvDef.c_str());

  // store old defs for formal args
  std::vector<std::string> oldFormalArgs;
  for (unsigned int j = 1; j < m_Args.size(); ++j)
    {
    oldFormalArgs.push_back(std::string());
    if (m_Makefile->GetDefinition(m_Args[j].c_str()))
      {
      oldFormalArgs[j-1] = m_Makefile->GetDefinition(m_Args[j].c_str());
      }
    m_Makefile->AddDefinition(m_Args[j].c_str(),expandedArgs[j-1].c_str());
    }
  
  // Invoke all the functions that were collected in the block.
  for(unsigned int c = 0; c < m_Functions.size(); ++c)
    {
    if(!m_Makefile->ExecuteCommand(m_Functions[c]))
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << args[0].FilePath << ":" << args[0].Line << "\n"
            << "A command failed during the invocation of macro \""
            << this->m_Args[0].c_str() << "\".\nThe failing line "
            << "in the macro definition was at\n" 
            << m_Functions[c].m_FilePath << ":"
            << m_Functions[c].m_Line << "\n";
      cmSystemTools::Error(error.str().c_str());
      return false;
      }
    }
  
  // restore all args
  m_Makefile->AddDefinition("ARGC",oldARGC.c_str());
  m_Makefile->AddDefinition("ARGN",oldARGN.c_str());
  m_Makefile->AddDefinition("ARGV",oldARGV.c_str());
  // restore old defs for formal args
  for (unsigned int j = 1; j < m_Args.size(); ++j)
    {
    m_Makefile->AddDefinition(m_Args[j].c_str(),oldFormalArgs[j-1].c_str());
    }
  // restore old defs for formal args
  for (unsigned int j = 0; j < oldARGVArgs.size(); ++j)
    {
    sprintf(argvName,"ARGV%i",j);
    m_Makefile->AddDefinition(argvName,oldARGVArgs[j].c_str());
    }
  
  return true;
}

bool cmMacroFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf) 
{
  // record commands until we hit the ENDMACRO
  // at the ENDMACRO call we shift gears and start looking for invocations
  if(cmSystemTools::LowerCase(lff.m_Name) == "endmacro")
    {
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(lff.m_Arguments, expandedArguments);
    if(!expandedArguments.empty() && (expandedArguments[0] == m_Args[0]))
      {
      std::string name = m_Args[0];
      std::vector<std::string>::size_type cc;
      name += "(";
      for ( cc = 0; cc < m_Args.size(); cc ++ )
        {
        name += " " + m_Args[cc];
        }
      name += " )";
      mf.AddMacro(m_Args[0].c_str(), name.c_str());
      
      // create a new command and add it to cmake
      cmMacroHelperCommand *f = new cmMacroHelperCommand();
      f->m_Args = this->m_Args;
      f->m_Functions = this->m_Functions;
      std::string newName = "_" + this->m_Args[0];
      mf.GetCMakeInstance()->RenameCommand(this->m_Args[0].c_str(), newName.c_str());
      mf.AddCommand(f);
      
      // remove the function blocker now that the macro is defined
      mf.RemoveFunctionBlocker(lff);
      return true;
      }
    }
  
  // if it wasn't an endmacro and we are not executing then we must be
  // recording
  m_Functions.push_back(lff);
  return true;
}
  

bool cmMacroFunctionBlocker::
ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf) 
{
  if(cmSystemTools::LowerCase(lff.m_Name) == "endmacro")
    {
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(lff.m_Arguments, expandedArguments);
    if(!expandedArguments.empty() && (expandedArguments[0] == m_Args[0]))
      {
      return true;
      }
    }
  return false;
}

void cmMacroFunctionBlocker::
ScopeEnded(cmMakefile &mf) 
{
  // macros should end with an EndMacro 
  cmSystemTools::Error("The end of a CMakeLists file was reached with a MACRO statement that was not closed properly. Within the directory: ", 
                       mf.GetCurrentDirectory(), " with macro ", 
                       m_Args[0].c_str());
}

bool cmMacroCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // create a function blocker
  cmMacroFunctionBlocker *f = new cmMacroFunctionBlocker();
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    f->m_Args.push_back(*j);
    }
  m_Makefile->AddFunctionBlocker(f);
  
  return true;
}

