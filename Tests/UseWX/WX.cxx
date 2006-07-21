//For wx
#include <wx/app.h>
#include <wx/dir.h>

static void TestDirEnumHelper(wxDir& dir,
                              int flags = wxDIR_DEFAULT,
                              const wxString& filespec = wxEmptyString)
{
    wxString filename;

    if ( !dir.IsOpened() )
        return;

    bool cont = dir.GetFirst(&filename, filespec, flags);
    while ( cont )
    {
        wxPrintf(_T("\t%s\n"), filename.c_str());

        cont = dir.GetNext(&filename);
    }

    wxPuts(_T(""));
}


//----------------------------------------------------------------------------
// MyApp
//----------------------------------------------------------------------------

class MyApp: public wxApp
{
public:
    MyApp();
    
    bool OnInit();
    int MainLoop();
};


IMPLEMENT_APP(MyApp)

MyApp::MyApp()
{
}

bool MyApp::OnInit()
{
    //test a directory that exist:
    wxDir dir(wxT("."));  //wxDir dir("/tmp");
    TestDirEnumHelper(dir, wxDIR_DEFAULT | wxDIR_DOTDOT);

    //Testing if link to wx debug library
#ifdef __WXDEBUG__
    printf("If you read this you're in __WXDEBUG__ debug mode.\n");
#endif  //__WXDEBUG__

#ifdef _DEBUG
    printf("If you read this then _DEBUG is defined.\n");
#endif  //_DEBUG

    wxChar ch = wxT('*');
    wxString s = wxT("Hello, world!");
    int len = s.Len();
    printf("Length of string is: %d\n", len);

    //Force testing of Unicode mode
#ifdef __UNICODE__
    wprintf(L"Unicode: %s \n", s.c_str());
    wprintf(:"Char: %c\n", ch);
#else
    printf("ANSI: %s \n", s.c_str());
    printf("Char: %c\n", ch);
#endif  //__UNICODE__

    //return immediately
    return TRUE;
}

int MyApp::MainLoop()
{
  return 0;
}
