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
      // Remove the function blocker for this scope or bail.
      cmsys::auto_ptr<cmFunctionBlocker>
        fb(mf.RemoveFunctionBlocker(this, lff));
      if(!fb.get()) { return false; }

      std::string errorString;
    
      std::vector<std::string> expandedArguments;
      mf.ExpandArguments(this->Args, expandedArguments);
      bool isTrue = 
        cmIfCommand::IsTrue(expandedArguments,errorString,&mf);

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
            return true;
            }
          if (status.GetBreakInvoked())
            {
            return true;
            }
          }
        expandedArguments.clear();
        mf.ExpandArguments(this->Args, expandedArguments);
        isTrue = 
          cmIfCommand::IsTrue(expandedArguments,errorString,&mf);
        }
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
ShouldRemove(const cmListFileFunction& lff, cmMakefile& )
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

