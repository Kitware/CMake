/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <QApplication>

class CMakeSetupDialog;

void SetupDefaultQSettings()
{
}

int CMakeGUIExec(CMakeSetupDialog* /*window*/)
{
  return QApplication::exec();
}
