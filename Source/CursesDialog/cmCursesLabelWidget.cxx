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
#include "cmCursesLabelWidget.h"

cmCursesLabelWidget::cmCursesLabelWidget(int width, int height, 
                                         int left, int top,
                                         const std::string& name) :
  cmCursesWidget(width, height, left, top)
{
  field_opts_off(m_Field,  O_EDIT);
  field_opts_off(m_Field,  O_ACTIVE);
  field_opts_off(m_Field,  O_STATIC);
  this->SetValue(name.c_str());
}

cmCursesLabelWidget::~cmCursesLabelWidget()
{
}

bool cmCursesLabelWidget::HandleInput(int&, cmCursesMainForm*, WINDOW* )
{
  // Static text. No input is handled here.
  return false;
}
