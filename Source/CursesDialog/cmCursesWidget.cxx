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
#include "cmCursesWidget.h"

cmCursesWidget::cmCursesWidget(int width, int height, int left, int top)
{
  m_Field = new_field(height, width, top, left, 0, 0);
  set_field_userptr(m_Field, reinterpret_cast<char*>(this));
  field_opts_off(m_Field,  O_AUTOSKIP);
  m_Page = 0;
}

cmCursesWidget::~cmCursesWidget()
{
  if (m_Field)
    {
    free_field(m_Field);
    m_Field = 0;
    }
}

void cmCursesWidget::Move(int x, int y, bool isNewPage)
{
  if (!m_Field)
    {
    return;
    }

  move_field(m_Field, y, x);
  if (isNewPage)
    {
    set_new_page(m_Field, TRUE);
    }
  else
    {
    set_new_page(m_Field, FALSE);
    }
}

void cmCursesWidget::SetValue(const char* value)
{
  m_Value = value;
  set_field_buffer(m_Field, 0, value);
}

const char* cmCursesWidget::GetValue()
{
  return m_Value.c_str();
}
