/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Terminal.h)

/* Work-around CMake dependency scanning limitation.  This must
   duplicate the above list of headers.  */
#if 0
# include "Terminal.h.in"
#endif

int main()
{
  kwsysTerminal_cfprintf(kwsysTerminal_Color_ForegroundYellow |
                         kwsysTerminal_Color_BackgroundBlue |
                         kwsysTerminal_Color_AssumeTTY,
                         stdout, "Hello %s!", "World");
  fprintf(stdout, "\n");
  return 0;
}
