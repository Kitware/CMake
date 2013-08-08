/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
      cmake::MessageType messageType;
      bool isTrue =
        cmIfCommand::IsTrue(expandedArguments,errorString,
                            &mf, messageType);

      while (isTrue)
        {
        if (errorString.size())
          {
          std::string err = "had incorrect arguments: ";
          unsigned int i;
          for(i =0; i < this->Args.size(); ++i)
            {
            err += (this->Args[i].Delim?"\"":"");
            err += this->Args[i].Value;
            err += (this->Args[i].Delim?"\"":"");
            err += " ";
            }
          err += "(";
          err += errorString;
          err += ").";
          mf.IssueMessage(messageType, err);
          if (messageType == cmake::FATAL_ERROR)
            {
            cmSystemTools::SetFatalErrorOccured();
            return true;
            }
          }

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
          if(cmSystemTools::GetFatalErrorOccured() )
            {
            return true;
            }
          }
        expandedArguments.clear();
        mf.ExpandArguments(this->Args, expandedArguments);
        isTrue =
          cmIfCommand::IsTrue(expandedArguments,errorString,
                              &mf, messageType);
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

