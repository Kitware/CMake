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

bool cmMacroFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf) 
{
  // record commands until we hit the ENDMACRO
  if (!m_Executing)
    {
    // at the ENDMACRO call we shift gears and start looking for invocations
    if(lff.m_Name == "ENDMACRO")
      {
      std::vector<std::string> expandedArguments;
      mf.ExpandArguments(lff.m_Arguments, expandedArguments);
      if(!expandedArguments.empty() && (expandedArguments[0] == m_Args[0]))
        {
        m_Executing = true;
        std::string name = m_Args[0];
        std::vector<std::string>::size_type cc;
        name += "(";
        for ( cc = 0; cc < m_Args.size(); cc ++ )
          {
          name += " " + m_Args[cc];
          }
        name += " )";
        mf.AddMacro(m_Args[0].c_str(), name.c_str());
        return true;
        }
      }
    // if it wasn't an endmacro and we are not executing then we must be
    // recording
    m_Functions.push_back(lff);
    return true;
    }
  
  // otherwise the macro has been recorded and we are executing
  // so we look for macro invocations
  if(lff.m_Name == m_Args[0])
    {
    std::string tmps;
    cmListFileArgument arg;
    std::string variable;
    // Expand the argument list to the macro.
    std::vector<std::string> expandedArguments;
    mf.ExpandArguments(lff.m_Arguments, expandedArguments);

    // make sure the number of arguments passed is at least the number
    // required by the signature
    if (expandedArguments.size() < m_Args.size() - 1)
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << lff.m_FilePath << ":" << lff.m_Line << ":\n"
            << "Invocation of macro \""
            << lff.m_Name.c_str() << "\" with incorrect number of arguments.";
      cmSystemTools::Error(error.str().c_str());
      return true;
      }

    // set the value of argc
    cmOStringStream argcDefStream;
    argcDefStream << expandedArguments.size();
    std::string argcDef = argcDefStream.str();
    
    // declare varuiables for ARGV ARGN but do not compute until needed
    std::string argvDef;
    std::string argnDef;
    bool argnDefInitialized = false;
    bool argvDefInitialized = false;
    
    // Invoke all the functions that were collected in the block.
    cmListFileFunction newLFF;
    // for each function
    for(unsigned int c = 0; c < m_Functions.size(); ++c)
      {
      // Replace the formal arguments and then invoke the command.
      newLFF.m_Arguments.clear();
      newLFF.m_Arguments.reserve(m_Functions[c].m_Arguments.size());
      newLFF.m_Name = m_Functions[c].m_Name;
      newLFF.m_FilePath = m_Functions[c].m_FilePath;
      newLFF.m_Line = m_Functions[c].m_Line;
      // for each argument of the current function
      for (std::vector<cmListFileArgument>::const_iterator k = 
             m_Functions[c].m_Arguments.begin();
           k != m_Functions[c].m_Arguments.end(); ++k)
        {
        tmps = k->Value;
        // replace formal arguments
        for (unsigned int j = 1; j < m_Args.size(); ++j)
          {
          variable = "${";
          variable += m_Args[j];
          variable += "}"; 
          cmSystemTools::ReplaceString(tmps, variable.c_str(),
                                       expandedArguments[j-1].c_str());
          }
        // replace argc
        cmSystemTools::ReplaceString(tmps, "${ARGC}",argcDef.c_str());
        
        // repleace ARGN
        if (tmps.find("${ARGN}") != std::string::npos)
          {
          if (!argnDefInitialized)
            {
            std::vector<std::string>::iterator eit;
            std::vector<std::string>::size_type cnt = 0;
            for ( eit = expandedArguments.begin();
                  eit != expandedArguments.end();
                  ++ eit )
              {
              if ( cnt >= m_Args.size()-1 )
                {
                if ( argnDef.size() > 0 )
                  {
                  argnDef += ";";
                  }
                argnDef += *eit;
                }
              cnt ++;
              }
            argnDefInitialized = true;
            }
          cmSystemTools::ReplaceString(tmps, "${ARGN}", argnDef.c_str());
          }
        
        // if the current argument of the current function has ${ARGV in it
        // then try replacing ARGV values
        if (tmps.find("${ARGV") != std::string::npos)
          {
          char argvName[60];
          
          // repleace ARGV, compute it only once
          if (!argvDefInitialized)
            {
            std::vector<std::string>::iterator eit;
            for ( eit = expandedArguments.begin();
                  eit != expandedArguments.end();
                  ++ eit )
              {
              if ( argvDef.size() > 0 )
                {
                argvDef += ";";
                }
              argvDef += *eit;
              }
            argvDefInitialized = true;
            }
          cmSystemTools::ReplaceString(tmps, "${ARGV}", argvDef.c_str());
          
          // also replace the ARGV1 ARGV2 ... etc
          for (unsigned int t = 0; t < expandedArguments.size(); ++t)
            {
            sprintf(argvName,"${ARGV%i}",t);
            cmSystemTools::ReplaceString(tmps, argvName,
                                         expandedArguments[t].c_str());
            }
          }
        
        arg.Value = tmps;
        arg.Quoted = k->Quoted;
        newLFF.m_Arguments.push_back(arg);
        }
      if(!mf.ExecuteCommand(newLFF))
        {
        cmOStringStream error;
        error << "Error in cmake code at\n"
              << lff.m_FilePath << ":" << lff.m_Line << ":\n"
              << "A command failed during the invocation of macro \""
              << lff.m_Name.c_str() << "\".";
        cmSystemTools::Error(error.str().c_str());
        }
      }
    return true;
    }

  // if not an invocation then it is just an ordinary line
  return false;
}

bool cmMacroFunctionBlocker::
ShouldRemove(const cmListFileFunction&, cmMakefile &) 
{
  return false;
}

void cmMacroFunctionBlocker::
ScopeEnded(cmMakefile &mf) 
{
  // macros never leave scope but we should have seen the ENDMACRO call by now
  if (m_Executing != true)
    {
    cmSystemTools::Error("The end of a CMakeLists file was reached with a MACRO statement that was not closed properly. Within the directory: ", 
                         mf.GetCurrentDirectory(), " with macro ", 
                         m_Args[0].c_str());
    }
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

