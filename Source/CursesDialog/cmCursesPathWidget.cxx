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
#include "cmDirectory.h"

cmCursesPathWidget::cmCursesPathWidget(int width, int height, 
                                           int left, int top) :
  cmCursesStringWidget(width, height, left, top)
{
  m_Type = cmCacheManager::PATH;
  m_Cycle = false;
  m_CurrentIndex = 0;
}

void GlobDirs(const std::string& fullPath,
              std::vector<std::string>& files,
              std::ofstream& of)
{
  if ( fullPath[fullPath.size()-1] != '*' )
    {
    files.push_back(fullPath);
    return;
    }
  std::string path = cmSystemTools::GetFilenamePath(fullPath);
  std::string ppath = cmSystemTools::GetFilenameName(fullPath);
  ppath = ppath.substr(0, ppath.size()-1);
  of << "Search in directory: " << path << std::endl;
  of << "Search pattern: " << ppath << std::endl;

  cmDirectory d;
  if (d.Load(path.c_str()))
    {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); ++i)
      {
      if((std::string(d.GetFile(i)) != ".")
         && (std::string(d.GetFile(i)) != ".."))
        {
        std::string fname = path;
        fname +="/";
        fname += d.GetFile(i);
        std::string sfname = d.GetFile(i);
        if(cmSystemTools::FileIsDirectory(fname.c_str()))
          {
          of << "Compare: " << sfname.substr(0, ppath.size()) << " and "
             << ppath << std::endl;
          if ( sfname.size() >= ppath.size() && 
               sfname.substr(0, ppath.size()) == 
               ppath )
            {
            files.push_back(fname);
            }
          }
        }
      }
    }
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
  std::ofstream of("lala.log");
  std::string::size_type cc;
  if ( !this->GetString() )
    {
    return;
    }
  FORM* form = fm->GetForm();
  form_driver(form, REQ_NEXT_FIELD);
  form_driver(form, REQ_PREV_FIELD);
  std::string cstr = this->GetString();
  cstr = cstr.substr(0, cstr.find_last_not_of(" \t\n\r")+1);
  of << "Cstr: " << cstr << " <> " << m_LastString << std::endl;
  if ( m_LastString != cstr )
    {
    m_Cycle = false;
    m_CurrentIndex = 0;
    m_LastGlob = "";
    of << "Reset" << std::endl;
    }
  std::string glob;
  if ( m_Cycle )
    {
    of << "We are cycling, try same glob" << std::endl;
    glob = m_LastGlob;
    }
  else
    {
    glob = cstr + "*";
    of << "Try new glob: " << glob << std::endl;
    }
  std::vector<std::string> dirs;

  ::GlobDirs(glob.c_str(), dirs, of);
  if ( m_CurrentIndex < dirs.size() )
    {
    cstr = dirs[m_CurrentIndex];
    }
  if ( cstr[cstr.size()-1] == '*' )
    {
    cstr = cstr.substr(0, cstr.size()-1);
    }

  of << "Glob: " << glob << std::endl;
  for ( cc =0; cc < dirs.size(); cc ++ )
    {
    of << "\t" << cc << ": " << dirs[cc] << std::endl;
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
