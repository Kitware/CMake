
#include "CMakeSetupGUIImplementation.h"
#include "FL/Fl.h"
#include "FL/fl_ask.H"

int main(int argc, char * argv[] ) 
{

  fl_message_font(FL_HELVETICA,11);

  CMakeSetupGUIImplementation * gui 
       = new CMakeSetupGUIImplementation;

  gui->SetPathToExecutable( argv[0] );
  gui->Show();
  gui->LoadCacheFromDiskToGUI();

  Fl::run();

  delete gui;
  
  return 0;
  
}
