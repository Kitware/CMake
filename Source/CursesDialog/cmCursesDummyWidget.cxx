/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCursesDummyWidget.h"

#include "cmCursesWidget.h"
#include "cmState.h"

cmCursesDummyWidget::cmCursesDummyWidget(int width, int height, int left,
                                         int top)
  : cmCursesWidget(width, height, left, top)
{
  this->Type = cmState::INTERNAL;
}

bool cmCursesDummyWidget::HandleInput(int& /*key*/, cmCursesMainForm* /*fm*/,
                                      WINDOW* /*w*/)
{
  return false;
}
