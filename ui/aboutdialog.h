#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>

class CAboutDialog : public wxDialog
{
    wxPanel* m_panel;
    wxStaticBitmap* m_logo;
    wxStaticText* m_appName;
    wxStaticText* m_appVersion;
    wxStaticText* m_appPurpose;
    wxStaticText* m_copyLabel;
    wxStaticText* m_copyright;
    wxStaticText* m_homeLabel;
    wxHyperlinkCtrl* m_homepage;
    wxButton* m_oK;

public:

    CAboutDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("About WebPier"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );

    ~CAboutDialog();
};

