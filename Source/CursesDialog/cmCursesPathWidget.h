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
#ifndef __cmCursesPathWidget_h
#define __cmCursesPathWidget_h

#include "cmCursesStringWidget.h"

class cmCursesPathWidget : public cmCursesStringWidget
{
public:
  cmCursesPathWidget(int width, int height, int left, int top);

protected:
  cmCursesPathWidget(const cmCursesPathWidget& from);
  void operator=(const cmCursesPathWidget&);

};

#endif // __cmCursesPathWidget_h
