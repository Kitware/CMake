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
#include "cmCursesStringWidget.h"
#include "cmCursesMainForm.h"

inline int ctrl(int z)
{
    return (z&037);
} 

cmCursesStringWidget::cmCursesStringWidget(int width, int height, 
                                           int left, int top) :
  cmCursesWidget(width, height, left, top)
{
  m_InEdit = false;
  m_Type = cmCacheManager::STRING;
  set_field_fore(m_Field,  A_NORMAL);
  set_field_back(m_Field,  A_STANDOUT);
  field_opts_off(m_Field,  O_STATIC);
}

void cmCursesStringWidget::OnTab(cmCursesMainForm*, WINDOW*)
{
  //FORM* form = fm->GetForm();
}

void cmCursesStringWidget::OnReturn(cmCursesMainForm* fm, WINDOW*)
{
  FORM* form = fm->GetForm();
  if (m_InEdit)
    {
    cmCursesForm::LogMessage("String widget leaving edit.");
    m_InEdit = false;
    fm->PrintKeys();
    delete[] m_OriginalString;
    // trick to force forms to update the field buffer
    form_driver(form, REQ_NEXT_FIELD);
    form_driver(form, REQ_PREV_FIELD);
    m_Done = true;
    }
  else
    {
    cmCursesForm::LogMessage("String widget entering edit.");
    m_InEdit = true;
    fm->PrintKeys();
    char* buf = field_buffer(m_Field, 0);
    m_OriginalString = new char[strlen(buf)+1];
    strcpy(m_OriginalString, buf);
    }
}

void cmCursesStringWidget::OnType(int& key, cmCursesMainForm* fm, WINDOW* w)
{
  form_driver(fm->GetForm(), key);
}

bool cmCursesStringWidget::HandleInput(int& key, cmCursesMainForm* fm, 
                                       WINDOW* w)
{
  int x,y;

  FORM* form = fm->GetForm();
  // 10 == enter
  if (!m_InEdit && ( key != 10 ) )
    {
    return false;
    }

  m_OriginalString=0;
  m_Done = false;

  char debugMessage[128];

  // <Enter> is used to change edit mode (like <Esc> in vi).
  while(!m_Done)
    {
    sprintf(debugMessage, "String widget handling input, key: %d", key);
    cmCursesForm::LogMessage(debugMessage);

    fm->PrintKeys();

    getmaxyx(stdscr, y, x);
    // If window too small, handle 'q' only
    if ( x < cmCursesMainForm::MIN_WIDTH  || 
         y < cmCursesMainForm::MIN_HEIGHT )
      {
      // quit
      if ( key == 'q' )
        {
        return false;
        }
      else
        {
        key=getch(); 
        continue;
        }
      }

    // If resize occured during edit, move out of edit mode
    if (!m_InEdit && ( key != 10 && key != KEY_ENTER ) )
      {
      return false;
      }
    // 10 == enter
    if (key == 10 || key == KEY_ENTER) 
      {
      this->OnReturn(fm, w);
      }
    else if ( key == KEY_DOWN || key == ctrl('n') ||
              key == KEY_UP || key == ctrl('p') ||
              key == KEY_NPAGE || key == ctrl('d') ||
              key == KEY_PPAGE || key == ctrl('u'))
      {
      m_InEdit = false;
      delete[] m_OriginalString;     
      // trick to force forms to update the field buffer
      form_driver(form, REQ_NEXT_FIELD);
      form_driver(form, REQ_PREV_FIELD);
      return false;
      }
    // esc
    else if (key == 27)
      {
      if (m_InEdit)
        {
        m_InEdit = false;
        fm->PrintKeys();
        this->SetString(m_OriginalString);
        delete[] m_OriginalString;
        touchwin(w); 
        wrefresh(w); 
        return true;
        }
      }
    else if ( key == 9 )
      {
      this->OnTab(fm, w);
      }
    else if ( key == KEY_LEFT || key == ctrl('b') )
      {
      form_driver(form, REQ_PREV_CHAR);
      }
    else if ( key == KEY_RIGHT || key == ctrl('f') )
      {
      form_driver(form, REQ_NEXT_CHAR);
      }
    else if ( key == ctrl('k') )
      {
      form_driver(form, REQ_CLR_EOL);
      }
    else if ( key == ctrl('a') )
      {
      form_driver(form, REQ_BEG_FIELD);
      }
    else if ( key == ctrl('e') )
      {
      form_driver(form, REQ_END_FIELD);
      }
    else if ( key == ctrl('d') || key == 127 || 
              key == KEY_BACKSPACE || key == KEY_DC )
      {
      form_driver(form, REQ_DEL_PREV);
      }
    else
      {
      this->OnType(key, fm, w);
      }
    if ( !m_Done )
      {
      touchwin(w); 
      wrefresh(w); 
      
      key=getch(); 
      }
    }
  return true;
}

void cmCursesStringWidget::SetString(const char* value)
{
  this->SetValue(value);
}

const char* cmCursesStringWidget::GetString()
{
  return this->GetValue();
}

const char* cmCursesStringWidget::GetValue()
{
  return field_buffer(m_Field, 0);
}

bool cmCursesStringWidget::PrintKeys()
{
  int x,y;
  getmaxyx(stdscr, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    return false;
    }
  if (m_InEdit)
    {
    char firstLine[512];
    // Clean the toolbar
    for(int i=0; i<512; i++)
      {
      firstLine[i] = ' ';
      }
    firstLine[511] = '\0';
    curses_move(y-4,0);
    printw(firstLine);
    curses_move(y-3,0);
    printw(firstLine);
    curses_move(y-2,0);
    printw(firstLine);
    curses_move(y-1,0);
    printw(firstLine);

    sprintf(firstLine,  "Editing option, press [enter] to leave edit.");
    curses_move(y-3,0);
    printw(firstLine);
    return true;
    }
  else
    {
    return false;
    }
}
