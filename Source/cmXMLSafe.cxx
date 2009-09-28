/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmXMLSafe.h"

#include <cmsys/ios/iostream>
#include <cmsys/ios/sstream>

#include <string.h>
#include <stdio.h>

//----------------------------------------------------------------------------
cmXMLSafe::cmXMLSafe(const char* s):
  Data(s),
  Size(static_cast<unsigned long>(strlen(s))),
  DoQuotes(true)
{
}

//----------------------------------------------------------------------------
cmXMLSafe::cmXMLSafe(cmsys_stl::string const& s):
    Data(s.c_str()),
    Size(static_cast<unsigned long>(s.length())),
    DoQuotes(true)
{
}

//----------------------------------------------------------------------------
cmXMLSafe& cmXMLSafe::Quotes(bool b)
{
  this->DoQuotes = b;
  return *this;
}

//----------------------------------------------------------------------------
cmsys_stl::string cmXMLSafe::str()
{
  cmsys_ios::ostringstream ss;
  ss << *this;
  return ss.str();
}

//----------------------------------------------------------------------------
cmsys_ios::ostream& operator<<(cmsys_ios::ostream& os, cmXMLSafe const& self)
{
  char const* first = self.Data;
  char const* last = self.Data + self.Size;
  for(char const* ci = first; ci != last; ++ci)
    {
    unsigned char c = static_cast<unsigned char>(*ci);
    switch(c)
      {
      case '&': os << "&amp;"; break;
      case '<': os << "&lt;"; break;
      case '>': os << "&gt;"; break;
      case '"': os << (self.DoQuotes? "&quot;" : "\""); break;
      case '\'': os << (self.DoQuotes? "&apos;" : "'"); break;
      case '\t': os << "\t"; break;
      case '\n': os << "\n"; break;
      case '\r': break; // Ignore CR
      default:
        if(c >= 0x20 && c <= 0x7f)
          {
          os.put(static_cast<char>(c));
          }
        else
          {
          // TODO: More complete treatment of program output character
          // encoding.  Instead of escaping these bytes, we should
          // handle the current locale and its encoding.
          char buf[16];
          // http://www.w3.org/TR/REC-xml/#NT-Char
          if(c >= 0x80)
            {
            sprintf(buf, "&#x%hx;", static_cast<unsigned short>(c));
            }
          else
            {
            // We cannot use "&#x%hx;" here because this value is not
            // valid in XML.  Instead use a human-readable hex value.
            sprintf(buf, "&lt;0x%hx&gt;", static_cast<unsigned short>(c));
            }
          os << buf;
          }
        break;
      }
    }
  return os;
}
