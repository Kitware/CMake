#ifndef  FLTKPropertyNameButtonWithHelp_h
#define  FLTKPropertyNameButtonWithHelp_h

#include <Fl/Fl_Tile.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Box.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Window.H>
#include <string>

namespace fltk {
  

/**
  
  Helper class for managing  help blobs over the property name

  */
class PropertyNameButtonWithHelp : public Fl_Button
{
  public:
    PropertyNameButtonWithHelp(int x,int y,int w, int h,const char *l);
    virtual ~PropertyNameButtonWithHelp();
    int handle(int event);
    void SetHelpText( const char * helpText);
    void ShowHelp(void);
    void HideHelp(void);
    
    static void ShowHelpBlobCallback( void * );

  private:
    
    string                m_HelpText;

    // Class variables
    static Fl_Window   *  helpBlob;
    static Fl_Box      *  helpText;
    static unsigned int   counter;
    static int            lastMousePositionX;
    static int            lastMousePositionY;

};




} // end namespace fltk

#endif


