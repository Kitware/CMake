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
#ifndef __cmCursesFilePathWidget_h
#define __cmCursesFilePathWidget_h

#include "cmCursesStringWidget.h"

class cmCursesFilePathWidget : public cmCursesStringWidget
{
public:
  cmCursesFilePathWidget(int width, int height, int left, int top);

protected:
  cmCursesFilePathWidget(const cmCursesFilePathWidget& from);
  void operator=(const cmCursesFilePathWidget&);

};

#endif // __cmCursesFilePathWidget_h
