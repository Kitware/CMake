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
#ifndef __cmCursesLabelWidget_h
#define __cmCursesLabelWidget_h

#include "cmCursesWidget.h"
#include "cmCursesStandardIncludes.h"

class cmCursesMainForm;

class cmCursesLabelWidget : public cmCursesWidget
{
public:
  cmCursesLabelWidget(int width, int height, int left, int top,
                      const std::string& name);
  virtual ~cmCursesLabelWidget();
  
  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled
  virtual bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w);

protected:
  cmCursesLabelWidget(const cmCursesLabelWidget& from);
  void operator=(const cmCursesLabelWidget&);
};

#endif // __cmCursesLabelWidget_h
