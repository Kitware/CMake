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
#include "cmSourceGroup.h"

//----------------------------------------------------------------------------
cmSourceGroup::cmSourceGroup(const char* name, const char* regex): Name(name)
{
  this->SetGroupRegex(regex);
}

//----------------------------------------------------------------------------
void cmSourceGroup::SetGroupRegex(const char* regex)
{
  if(regex)
    {
    this->GroupRegex.compile(regex);
    }
  else
    {
    this->GroupRegex.compile("^$");
    }
}
  
//----------------------------------------------------------------------------
void cmSourceGroup::AddGroupFile(const char* name)
{
  this->GroupFiles.insert(name);
}
  
//----------------------------------------------------------------------------
const char* cmSourceGroup::GetName() const
{
  return this->Name.c_str();
}
  
//----------------------------------------------------------------------------
bool cmSourceGroup::MatchesRegex(const char* name)
{
  return this->GroupRegex.find(name);
}

//----------------------------------------------------------------------------
bool cmSourceGroup::MatchesFiles(const char* name)
{
  std::set<cmStdString>::const_iterator i = this->GroupFiles.find(name);
  if(i != this->GroupFiles.end())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void cmSourceGroup::AssignSource(const cmSourceFile* sf)
{
  this->SourceFiles.push_back(sf);
}

//----------------------------------------------------------------------------
const std::vector<const cmSourceFile*>& cmSourceGroup::GetSourceFiles() const
{
  return this->SourceFiles;
}

//----------------------------------------------------------------------------
std::vector<const cmSourceFile*>& cmSourceGroup::GetSourceFiles()
{
  return this->SourceFiles;
}

//----------------------------------------------------------------------------
void cmSourceGroup::AddChild(cmSourceGroup child)
{
  this->GroupChildren.push_back(child);
}

//----------------------------------------------------------------------------
cmSourceGroup *cmSourceGroup::lookupChild(const char* name)
{
  // initializing iterators
  std::vector<cmSourceGroup>::iterator iter = this->GroupChildren.begin();
  std::vector<cmSourceGroup>::iterator end = this->GroupChildren.end();

  // st
  for(;iter!=end; ++iter)
    {
    std::string sgName = iter->GetName(); 

    // look if descenened is the one were looking for
    if(sgName == name)
      {
      return &(*iter); // if it so return it 
      }
    // if the descendend isn't the one where looking for ask it's traverse
    cmSourceGroup *result = iter->lookupChild(name);
                
    // if one of it's descendeds is the one we're looking for return it 
    if(result)
      {
      return result;
      }
    }

  // if no child with this name was found return NULL
  return NULL;
}

cmSourceGroup *cmSourceGroup::MatchChildrenFiles(const char *name)
{
  // initializing iterators
  std::vector<cmSourceGroup>::iterator iter = this->GroupChildren.begin();
  std::vector<cmSourceGroup>::iterator end = this->GroupChildren.end();

  if(this->MatchesFiles(name))
    {
    return this;
    }
  for(;iter!=end;++iter)
    {
    cmSourceGroup *result = iter->MatchChildrenFiles(name);
    if(result)
      {
      return result;
      }
    }
  return 0;
}


cmSourceGroup *cmSourceGroup::MatchChildrenRegex(const char *name)
{
  // initializing iterators
  std::vector<cmSourceGroup>::iterator iter = this->GroupChildren.begin();
  std::vector<cmSourceGroup>::iterator end = this->GroupChildren.end();

  if(this->MatchesRegex(name))
    {
    return this;
    }
  for(;iter!=end; ++iter)
    {
    cmSourceGroup *result = iter->MatchChildrenRegex(name);
    if(result)
      {
      return result;
      }
    }
  return 0;
}

std::vector<cmSourceGroup> cmSourceGroup::GetGroupChildren() const
{
  return this->GroupChildren;
}
