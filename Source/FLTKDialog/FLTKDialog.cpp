
#include "CMakeSetupGUIImplementation.h"
#include "FL/FL.h"
#include "FL/FL_ask.H"

int main() 
{

  fl_message_font(FL_HELVETICA,11);

  CMakeSetupGUIImplementation * gui 
       = new CMakeSetupGUIImplementation;

  gui->Show();

  Fl::run();

  delete gui;
  
  return 0;
  
}
