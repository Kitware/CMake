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
#include "cmCursesPathWidget.h"

#include "cmCursesMainForm.h"
#include "cmSystemTools.h"

cmCursesPathWidget::cmCursesPathWidget(int width, int height, 
                                           int left, int top) :
  cmCursesStringWidget(width, height, left, top)
{
  m_Type = cmCacheManager::PATH;
  m_Cycle = false;
  m_CurrentIndex = 0;
}

void cmCursesPathWidget::OnType(int& key, cmCursesMainForm* fm, WINDOW* w)
{
  m_Cycle = false;
  m_CurrentIndex = 0;
  m_LastGlob = "";
  this->cmCursesStringWidget::OnType(key, fm, w);
}

void cmCursesPathWidget::OnTab(cmCursesMainForm* fm, WINDOW* w)
{
  if ( !this->GetString() )
    {
    return;
    }
  FORM* form = fm->GetForm();
  form_driver(form, REQ_NEXT_FIELD);
  form_driver(form, REQ_PREV_FIELD);
  std::string cstr = this->GetString();
  cstr = cstr.substr(0, cstr.find_last_not_of(" \t\n\r")+1);
  if ( m_LastString != cstr )
    {
    m_Cycle = false;
    m_CurrentIndex = 0;
    m_LastGlob = "";
    }
  std::string glob;
  if ( m_Cycle )
    {
    glob = m_LastGlob;
    }
  else
    {
    glob = cstr + "*";
    }
  std::vector<cmStdString> dirs;

  cmSystemTools::SimpleGlob(glob.c_str(), dirs, (m_Type == cmCacheManager::PATH?-1:0));
  if ( m_CurrentIndex < dirs.size() )
    {
    cstr = dirs[m_CurrentIndex];
    }
  if ( cstr[cstr.size()-1] == '*' )
    {
    cstr = cstr.substr(0, cstr.size()-1);
    }

  if ( cmSystemTools::FileIsDirectory(cstr.c_str()) )
    {
    cstr += "/";
    }

  this->SetString(cstr.c_str());
  touchwin(w); 
  wrefresh(w); 
  form_driver(form, REQ_END_FIELD);
  m_LastGlob = glob;
  m_LastString = cstr;
  m_Cycle = true;
  m_CurrentIndex ++;
  if ( m_CurrentIndex >= dirs.size() )
    {
    m_CurrentIndex = 0;
    }
}

void cmCursesPathWidget::OnReturn(cmCursesMainForm* fm, WINDOW* w)
{
  this->cmCursesStringWidget::OnReturn(fm, w);
}
