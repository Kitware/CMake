/*=========================================================================

  Program:   WXDialog - wxWidgets X-platform GUI Front-End for CMake
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Author:    Jorgen Bodde

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __CMAKE_CONFIG_H__
#define __CMAKE_CONFIG_H__

#define CM_LASTPROJECT_PATH   "LastProjectPath"         // last project path for CMakeLists.txt in edit box
#define CM_LASTBUILD_PATH     "LastBuildPath"           // last build path for cache in edit box
#define CM_MAX_RECENT_PATHS   5                         // max amount most recent used paths in menu
#define CM_MAX_SEARCH_QUERIES 10                        // max amount of entries in search query window
#define CM_RECENT_BUILD_PATH  "LastRecentPath"          // will be enumerated like LastRecentPath0 LastRecentPath1 etc
#define CM_SEARCH_QUERY       "SearchQuery"             // will be enumerated like SearchQuery0 SearchQuery1 etc
#define CM_XSIZE              "CMakeXSize"              // x size of window
#define CM_YSIZE              "CMakeYSize"              // y size of window
#define CM_XPOS               "CMakeXPos"               // x pos of window
#define CM_YPOS               "CMakeYPos"               // y pos of window
#define CM_SPLITTERPOS        "CMakeSplitterPos"        // position of splitter window

#define CM_CLOSEAFTERGEN      "CMakeCloseAfterGenerate" // close CMake after succesful generation (old behaviour)
#define CM_CLOSEAFTERGEN_DEF  false                     // inherit default false when not present

#define CM_RECENT_BUILD_ITEM  10000                     // ID of menu item for recent builds
#define CMAKEGUI_MAJORVER     0                         // major build
#define CMAKEGUI_MINORVER     9                         // minor build
#define CMAKEGUI_ADDVER       "b"                       // postfix (beta, alpha, none)

#endif
