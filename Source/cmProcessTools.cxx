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
#include "cmProcessTools.h"

#include <cmsys/Process.h>

//----------------------------------------------------------------------------
void cmProcessTools::RunProcess(struct cmsysProcess_s* cp,
                                OutputParser* out, OutputParser* err)
{
  cmsysProcess_Execute(cp);
  char* data = 0;
  int length = 0;
  int p;
  while((out||err) && (p=cmsysProcess_WaitForData(cp, &data, &length, 0), p))
    {
    if(out && p == cmsysProcess_Pipe_STDOUT)
      {
      if(!out->Process(data, length))
        {
        out = 0;
        }
      }
    else if(err && p == cmsysProcess_Pipe_STDERR)
      {
      if(!err->Process(data, length))
        {
        err = 0;
        }
      }
    }
  cmsysProcess_WaitForExit(cp, 0);
}


//----------------------------------------------------------------------------
cmProcessTools::LineParser::LineParser(char sep, bool ignoreCR):
  Separator(sep), IgnoreCR(ignoreCR), Log(0), Prefix(0)
{
}

//----------------------------------------------------------------------------
void cmProcessTools::LineParser::SetLog(std::ostream* log, const char* prefix)
{
  this->Log = log;
  this->Prefix = prefix? prefix : "";
}

//----------------------------------------------------------------------------
bool cmProcessTools::LineParser::ProcessChunk(const char* first, int length)
{
  const char* last = first + length;
  for(const char* c = first; c != last; ++c)
    {
    if(*c == this->Separator)
      {
      // Log this line.
      if(this->Log && this->Prefix)
        {
        *this->Log << this->Prefix << this->Line << "\n";
        }

      // Hand this line to the subclass implementation.
      if(!this->ProcessLine())
        {
        this->Line = "";
        return false;
        }

      this->Line = "";
      }
    else if(*c != '\r' || !this->IgnoreCR)
      {
      // Append this character to the line under construction.
      this->Line.append(1, *c);
      }
    }
  return true;
}
