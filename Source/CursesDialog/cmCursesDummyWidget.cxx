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
#include "cmCursesDummyWidget.h"

cmCursesDummyWidget::cmCursesDummyWidget(int width, int height, 
                                           int left, int top) :
  cmCursesWidget(width, height, left, top)
{
  m_Type = cmCacheManager::INTERNAL;
}


bool cmCursesDummyWidget::HandleInput(int&, cmCursesMainForm*, WINDOW* )
{
  return false;
}

