
#include "CMakeSetupGUIImplementation.h"
#include "FL/Fl.h"
#include "FL/fl_ask.H"

int main() 
{

  fl_message_font(FL_HELVETICA,11);

  CMakeSetupGUIImplementation * gui 
       = new CMakeSetupGUIImplementation;

  gui->Show();
  gui->LoadCacheFromDiskToGUI();

  Fl::run();

  delete gui;
  
  return 0;
  
}
