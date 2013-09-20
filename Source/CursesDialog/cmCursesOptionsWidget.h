/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef __cmCursesOptionsWidget_h
#define __cmCursesOptionsWidget_h

#include "cmCursesWidget.h"
class cmCursesMainForm;

class cmCursesOptionsWidget : public cmCursesWidget
{
public:
  cmCursesOptionsWidget(int width, int height, int left, int top);

  // Description:
  // Handle user input. Called by the container of this widget
  // when this widget has focus. Returns true if the input was
  // handled.
  virtual bool HandleInput(int& key, cmCursesMainForm* fm, WINDOW* w);
  void SetOption(const char*);
  void AddOption(std::string const &);
  void NextOption();
  void PreviousOption();
protected:
  cmCursesOptionsWidget(const cmCursesOptionsWidget& from);
  void operator=(const cmCursesOptionsWidget&);
  std::vector<std::string> Options;
  unsigned int CurrentOption;
};

#endif // __cmCursesOptionsWidget_h
