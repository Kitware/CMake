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
#ifndef __cmCursesBoolWidget_h
#define __cmCursesBoolWidget_h

#include "cmCursesWidget.h"
class cmCursesMainForm;

class cmCursesBoolWidget : public cmCursesWidget
{
public:
  cmCursesBoolWidget(int width, int height, int left, int top);
  
  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled.
  virtual bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w);

  // Description:
  // Set/Get the value (on/off).
  void SetValueAsBool(bool value);
  bool GetValueAsBool();

protected:
  cmCursesBoolWidget(const cmCursesBoolWidget& from);
  void operator=(const cmCursesBoolWidget&);

};

#endif // __cmCursesBoolWidget_h
