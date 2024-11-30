#pragma once

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/colour.h>
#include <wx/dialog.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

class CStartupDialog : public wxDialog
{
    wxStaticBitmap* m_logo;
    wxStaticText* m_message;
    wxTextCtrl* m_ownerCtrl;
    wxTextCtrl* m_pierCtrl;

    void onOkButtonClick(wxCommandEvent& event);
    void onCloseButtonClick(wxCloseEvent& event);

public:

    CStartupDialog(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& title = _("Startup"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE);

    ~CStartupDialog();

    wxString GetIdentity() const
    {
        return m_ownerCtrl->GetValue() + "/" + m_pierCtrl->GetValue();
    }
};
