/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmIDEOptions.h"

#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmIDEOptions::cmIDEOptions()
{
  this->DoingDefine = false;
  this->AllowDefine = true;
  this->AllowSlash = false;
  for(int i=0; i < FlagTableCount; ++i)
    {
    this->FlagTable[i] = 0;
    }
}

//----------------------------------------------------------------------------
cmIDEOptions::~cmIDEOptions()
{
}

//----------------------------------------------------------------------------
void cmIDEOptions::HandleFlag(const char* flag)
{
  // If the last option was -D then this option is the definition.
  if(this->DoingDefine)
    {
    this->DoingDefine = false;
    this->Defines.push_back(flag);
    return;
    }

  // Look for known arguments.
  if(flag[0] == '-' || (this->AllowSlash && flag[0] == '/'))
    {
    // Look for preprocessor definitions.
    if(this->AllowDefine && flag[1] == 'D')
      {
      if(flag[2] == '\0')
        {
        // The next argument will have the definition.
        this->DoingDefine = true;
        }
      else
        {
        // Store this definition.
        this->Defines.push_back(flag+2);
        }
      return;
      }

    // Look through the available flag tables.
    bool flag_handled = false;
    for(int i=0; i < FlagTableCount && this->FlagTable[i]; ++i)
      {
      if(this->CheckFlagTable(this->FlagTable[i], flag, flag_handled))
        {
        return;
        }
      }

    // If any map entry handled the flag we are done.
    if(flag_handled)
      {
      return;
      }
    }

  // This option is not known.  Store it in the output flags.
  this->StoreUnknownFlag(flag);
}

//----------------------------------------------------------------------------
bool cmIDEOptions::CheckFlagTable(cmIDEFlagTable const* table,
                                  const char* flag, bool& flag_handled)
{
  // Look for an entry in the flag table matching this flag.
  for(cmIDEFlagTable const* entry = table; entry->IDEName; ++entry)
    {
    bool entry_found = false;
    if(entry->special & cmIDEFlagTable::UserValue)
      {
      // This flag table entry accepts a user-specified value.  If
      // the entry specifies UserRequired we must match only if a
      // non-empty value is given.
      int n = static_cast<int>(strlen(entry->commandFlag));
      if(strncmp(flag+1, entry->commandFlag, n) == 0 &&
         (!(entry->special & cmIDEFlagTable::UserRequired) ||
          static_cast<int>(strlen(flag+1)) > n))
        {
        if(entry->special & cmIDEFlagTable::UserIgnored)
          {
          // Ignore the user-specified value.
          this->FlagMap[entry->IDEName] = entry->value;
          }
        else if(entry->special & cmIDEFlagTable::SemicolonAppendable)
          {
          const char *new_value = flag+1+n;

          std::map<cmStdString,cmStdString>::iterator itr;
          itr = this->FlagMap.find(entry->IDEName);
          if(itr != this->FlagMap.end())
            {
            // Append to old value (if present) with semicolons;
            itr->second += ";";
            itr->second += new_value;
            }
          else
            {
            this->FlagMap[entry->IDEName] = new_value;
            }
          }
        else
          {
          // Use the user-specified value.
          this->FlagMap[entry->IDEName] = flag+1+n;
          }
        entry_found = true;
        }
      }
    else if(strcmp(flag+1, entry->commandFlag) == 0)
      {
      // This flag table entry provides a fixed value.
      this->FlagMap[entry->IDEName] = entry->value;
      entry_found = true;
      }

    // If the flag has been handled by an entry not requesting a
    // search continuation we are done.
    if(entry_found && !(entry->special & cmIDEFlagTable::Continue))
      {
      return true;
      }

    // If the entry was found the flag has been handled.
    flag_handled = flag_handled || entry_found;
    }

  return false;
}

//----------------------------------------------------------------------------
void cmIDEOptions::AddDefine(const std::string& def)
{
  this->Defines.push_back(def);
}

//----------------------------------------------------------------------------
void cmIDEOptions::AddDefines(const char* defines)
{
  if(defines)
    {
    // Expand the list of definitions.
    cmSystemTools::ExpandListArgument(defines, this->Defines);
    }
}

//----------------------------------------------------------------------------
void cmIDEOptions::AddFlag(const char* flag, const char* value)
{
  this->FlagMap[flag] = value;
}

//----------------------------------------------------------------------------
void cmIDEOptions::RemoveFlag(const char* flag)
{
  this->FlagMap.erase(flag);
}
