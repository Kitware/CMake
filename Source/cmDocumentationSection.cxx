/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmDocumentationSection.h"


//----------------------------------------------------------------------------
void cmDocumentationSection::Append(const char *data[][3])
{
  int i = 0;
  while(data[i][1])
    {
    this->Entries.push_back(cmDocumentationEntry(data[i][0],
                                                 data[i][1],
                                                 data[i][2]));
    data += 1;
    }
}

//----------------------------------------------------------------------------
void cmDocumentationSection::Prepend(const char *data[][3])
{
  std::vector<cmDocumentationEntry> tmp;
  int i = 0;
  while(data[i][1])
    {
    tmp.push_back(cmDocumentationEntry(data[i][0],
                                       data[i][1],
                                       data[i][2]));
    data += 1;
    }
  this->Entries.insert(this->Entries.begin(),tmp.begin(),tmp.end());
}

//----------------------------------------------------------------------------
void cmDocumentationSection::Append(const char *n, const char *b,
                                    const char *f)
{
  this->Entries.push_back(cmDocumentationEntry(n,b,f));
}

#if 0
//----------------------------------------------------------------------------
void cmDocumentationSection::Set(const cmDocumentationEntry* header,
                                 const cmDocumentationEntry* section,
                                 const cmDocumentationEntry* footer)
{
  this->Entries.erase(this->Entries.begin(), this->Entries.end());
  if(header)
    {
    for(const cmDocumentationEntry* op = header; op->brief; ++op)
      {
      this->Entries.push_back(*op);
      }
    }
  if(section)
    {
    for(const cmDocumentationEntry* op = section; op->brief; ++op)
      {
      this->Entries.push_back(*op);
      }
    }
  if(footer)
    {
    for(const cmDocumentationEntry* op = footer; op->brief; ++op)
      {
      this->Entries.push_back(*op);
      }
    }
  cmDocumentationEntry empty = {0,0,0};
  this->Entries.push_back(empty);
}
#endif
