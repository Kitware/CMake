/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#ifndef __cmDirectory_h
#define __cmDirectory_h

#include <iostream>
#include <string>
#include <vector>


/** \class cmDirectory
 * \brief Portable directory/filename traversal.
 * 
 * cmDirectory provides a portable way of finding the names of the files
 * in a system directory.
 *
 * cmDirectory works with windows and unix only.
 */


class cmDirectory 
{
public:
  
  /**
   * Load the specified directory and load the names of the files
   * in that directory. 0 is returned if the directory can not be 
   * opened, 1 if it is opened.   
   */
  bool Load(const char* dir);

  /**
   * Return the number of files in the current directory.
   */
  int GetNumberOfFiles() { return m_Files.size();}

  /**
   * Return the file at the given index, the indexing is 0 based
   */
  const char* GetFile(unsigned int index);

private:
  std::vector<std::string> m_Files; // Array of Files
  std::string m_Path;               // Path to Open'ed directory
}; // End Class: cmDirectory

  
#endif
