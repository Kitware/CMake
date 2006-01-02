/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCPackSTGZGenerator_h
#define cmCPackSTGZGenerator_h


#include "cmCPackTGZGenerator.h"

/** \class cmCPackSTGZGenerator
 * \brief A generator for Self extractable TGZ files
 *
 */
class cmCPackSTGZGenerator : public cmCPackTGZGenerator
{
public:
  cmCPackTypeMacro(cmCPackSTGZGenerator, cmCPackTGZGenerator);

  /**
   * Construct generator
   */
  cmCPackSTGZGenerator();
  virtual ~cmCPackSTGZGenerator();

protected:
  int GenerateHeader(std::ostream* os);
  virtual const char* GetOutputExtension() { return "sh"; }
};

#endif



