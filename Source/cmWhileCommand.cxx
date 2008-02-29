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
#include "cmWhileCommand.h"
#include "cmIfCommand.h"

bool cmWhileFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& lff, cmMakefile &mf,
                  cmExecutionStatus &inStatus)
{
  // Prevent recusion and don't let this blocker block its own
  // commands.
  if (this->Executing)
    {
    return false;
    }
  
  // at end of for each execute recorded commands
  if (!cmSystemTools::Strucmp(lff.Name.c_str(),"while"))
    {
    // record the number of while commands past this one
    this->Depth++;
    }
  else if (!cmSystemTools::Strucmp(lff.Name.c_str(),"endwhile"))
    {
    // if this is the endwhile for this while loop then execute
    if (!this->Depth) 
      {
      char* errorString = 0;
    
      std::vector<std::string> expandedArguments;
      mf.ExpandArguments(this->Args, expandedArguments);
      bool isTrue = 
        cmIfCommand::IsTrue(expandedArguments,&errorString,&mf);

      this->Executing = true;
      while (isTrue)
        {      
        // Invoke all the functions that were collected in the block.
        for(unsigned int c = 0; c < this->Functions.size(); ++c)
          {
          cmExecutionStatus status;
          mf.ExecuteCommand(this->Functions[c],status);
          if (status.GetReturnInvoked())
            {
            inStatus.SetReturnInvoked(true);
            mf.RemoveFunctionBlocker(lff);
            return true;
            }
          if (status.GetBreakInvoked())
            {
            mf.RemoveFunctionBlocker(lff);
            return true;
            }
          }
        expandedArguments.clear();
        mf.ExpandArguments(this->Args, expandedArguments);
        isTrue = 
          cmIfCommand::IsTrue(expandedArguments,&errorString,&mf);
        }
      mf.RemoveFunctionBlocker(lff);
      return true;
      }
    else
      {
      // decrement for each nested while that ends
      this->Depth--;
      }
    }

  // record the command
  this->Functions.push_back(lff);
  
  // always return true
  return true;
}

bool cmWhileFunctionBlocker::
ShouldRemove(const cmListFileFunction& lff, cmMakefile& mf)
{
  if(!cmSystemTools::Strucmp(lff.Name.c_str(),"endwhile"))
    {
    // if the endwhile has arguments, then make sure
    // they match the arguments of the matching while
    if (lff.Arguments.size() == 0 ||
        lff.Arguments == this->Args)
      {
      return true;
      }
    }
  return false;
}

void cmWhileFunctionBlocker::
ScopeEnded(cmMakefile &mf) 
{
  cmSystemTools::Error(
    "The end of a CMakeLists file was reached with a WHILE statement that "
    "was not closed properly. Within the directory: ", 
    mf.GetCurrentDirectory());
}

bool cmWhileCommand
::InvokeInitialPass(const std::vector<cmListFileArgument>& args, 
                    cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // create a function blocker
  cmWhileFunctionBlocker *f = new cmWhileFunctionBlocker();
  f->Args = args;
  this->Makefile->AddFunctionBlocker(f);
  
  return true;
}

