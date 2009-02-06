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
#ifndef cmXMLSafe_h
#define cmXMLSafe_h

#include <cmsys/stl/string>
#include <cmsys/ios/iosfwd>

/** \class cmXMLSafe
 * \brief Write strings to XML with proper escapes
 */
class cmXMLSafe
{
public:
  /** Construct with the data to be written.  This assumes the data
      will exist for the duration of this object's life.  */
  cmXMLSafe(const char* s);
  cmXMLSafe(cmsys_stl::string const& s);

  /** Specify whether to escape quotes too.  This is needed when
      writing the content of an attribute value.  By default quotes
      are escaped.  */
  cmXMLSafe& Quotes(bool b = true);

  /** Get the escaped data as a string.  */
  cmsys_stl::string str();
private:
  char const* Data;
  unsigned long Size;
  bool DoQuotes;
  friend cmsys_ios::ostream& operator<<(cmsys_ios::ostream&,
                                        cmXMLSafe const&);
};

#endif
