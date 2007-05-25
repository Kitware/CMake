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
#ifndef cmHexFileConverter_h
#define cmHexFileConverter_h

/** \class cmHexFileConverter
 * \brief Can detects Intel Hex and Motorola S-record files and convert them 
 *        to binary files.
 *
 */
class cmHexFileConverter
{
public:
  enum FileType {Binary, IntelHex, MotorolaSrec};
  static FileType DetermineFileType(const char* inFileName);
  static bool TryConvert(const char* inFileName, const char* outFileName);
};

#endif
