#ifndef  FLTKPropertyNameButtonWithHelp_h
#define  FLTKPropertyNameButtonWithHelp_h

#include <FL/Fl_Tile.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
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

    void PopupMenu( void );

  private:
    
    std::string           m_HelpText;

    // Class variables
    static Fl_Window   *  helpBlob;
    static Fl_Box      *  helpText;
    static unsigned int   counter;
    static int            lastMousePositionX;
    static int            lastMousePositionY;

};




} // end namespace fltk

#endif


