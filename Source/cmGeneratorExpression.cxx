/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGeneratorExpression.h"

#include "cmMakefile.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
cmGeneratorExpression::cmGeneratorExpression(
  cmMakefile* mf, const char* config,
  cmListFileBacktrace const& backtrace, bool quiet):
  Makefile(mf), Config(config), Backtrace(backtrace), Quiet(quiet)
{
  this->TargetInfo.compile("^\\$<TARGET"
                           "(|_SONAME|_LINKER)"  // File with what purpose?
                           "_FILE(|_NAME|_DIR):" // Filename component.
                           "([A-Za-z0-9_.-]+)"   // Target name.
                           ">$");
}

//----------------------------------------------------------------------------
const char* cmGeneratorExpression::Process(std::string const& input)
{
  return this->Process(input.c_str());
}

//----------------------------------------------------------------------------
const char* cmGeneratorExpression::Process(const char* input)
{
  this->Data.clear();

  // We construct and evaluate expressions directly in the output
  // buffer.  Each expression is replaced by its own output value
  // after evaluation.  A stack of barriers records the starting
  // indices of open (pending) expressions.
  for(const char* c = input; *c; ++c)
    {
    if(c[0] == '$' && c[1] == '<')
      {
      this->Barriers.push(this->Data.size());
      this->Data.push_back('$');
      this->Data.push_back('<');
      c += 1;
      }
    else if(c[0] == '>' && !this->Barriers.empty())
      {
      this->Data.push_back('>');
      if(!this->Evaluate()) { break; }
      this->Barriers.pop();
      }
    else
      {
      this->Data.push_back(c[0]);
      }
    }

  // Return a null-terminated output value.
  this->Data.push_back('\0');
  return &*this->Data.begin();
}

//----------------------------------------------------------------------------
bool cmGeneratorExpression::Evaluate()
{
  // The top-most barrier points at the beginning of the expression.
  size_t barrier = this->Barriers.top();

  // Construct a null-terminated representation of the expression.
  this->Data.push_back('\0');
  const char* expr = &*(this->Data.begin()+barrier);

  // Evaluate the expression.
  std::string result;
  if(this->Evaluate(expr, result))
    {
    // Success.  Replace the expression with its evaluation result.
    this->Data.erase(this->Data.begin()+barrier, this->Data.end());
    this->Data.insert(this->Data.end(), result.begin(), result.end());
    return true;
    }
  else if(!this->Quiet)
    {
    // Failure.  Report the error message.
    cmOStringStream e;
    e << "Error evaluating generator expression:\n"
      << "  " << expr << "\n"
      << result;
    this->Makefile->GetCMakeInstance()
      ->IssueMessage(cmake::FATAL_ERROR, e.str().c_str(),
                     this->Backtrace);
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmGeneratorExpression::Evaluate(const char* expr, std::string& result)
{
  if(this->TargetInfo.find(expr))
    {
    if(!this->EvaluateTargetInfo(result))
      {
      return false;
      }
    }
  else if(strcmp(expr, "$<CONFIGURATION>") == 0)
    {
    result = this->Config? this->Config : "";
    }
  else
    {
    result = "Expression syntax not recognized.";
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmGeneratorExpression::EvaluateTargetInfo(std::string& result)
{
  // Lookup the referenced target.
  std::string name = this->TargetInfo.match(3);
  cmTarget* target = this->Makefile->FindTargetToUse(name.c_str());
  if(!target)
    {
    result = "No target \"" + name + "\"";
    return false;
    }
  if(target->GetType() >= cmTarget::UTILITY &&
     target->GetType() != cmTarget::UNKNOWN_LIBRARY)
    {
    result = "Target \"" + name + "\" is not an executable or library.";
    return false;
    }
  this->Targets.insert(target);

  // Lookup the target file with the given purpose.
  std::string purpose = this->TargetInfo.match(1);
  if(purpose == "")
    {
    // The target implementation file (.so.1.2, .dll, .exe, .a).
    result = target->GetFullPath(this->Config, false, true);
    }
  else if(purpose == "_LINKER")
    {
    // The file used to link to the target (.so, .lib, .a).
    if(!target->IsLinkable())
      {
      result = ("TARGET_LINKER_FILE is allowed only for libraries and "
                "executables with ENABLE_EXPORTS.");
      return false;
      }
    result = target->GetFullPath(this->Config, target->HasImportLibrary());
    }
  else if(purpose == "_SONAME")
    {
    // The target soname file (.so.1).
    if(target->IsDLLPlatform())
      {
      result = "TARGET_SONAME_FILE is not allowed for DLL target platforms.";
      return false;
      }
    if(target->GetType() != cmTarget::SHARED_LIBRARY)
      {
      result = "TARGET_SONAME_FILE is allowed only for SHARED libraries.";
      return false;
      }
    result = target->GetDirectory(this->Config);
    result += "/";
    result += target->GetSOName(this->Config);
    }

  // Extract the requested portion of the full path.
  std::string part = this->TargetInfo.match(2);
  if(part == "_NAME")
    {
    result = cmSystemTools::GetFilenameName(result);
    }
  else if(part == "_DIR")
    {
    result = cmSystemTools::GetFilenamePath(result);
    }
  return true;
}
