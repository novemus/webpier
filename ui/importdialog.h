#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/checklst.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

class CImportDialog : public wxDialog
{
    wxCheckListBox* m_serviceList;
    wxStaticText* m_idLabel;
    wxStaticText* m_idValue;
    wxStaticText* m_serviceLabel;
    wxTextCtrl* m_serviceCtrl;
    wxStaticText* m_gateLabel;
    wxTextCtrl* m_gateCtrl;
    wxStaticText* m_startLabel;
    wxCheckBox* m_startCtrl;
    wxStaticText* m_obscureLabel;
    wxStaticText* m_obscureValue;
    wxStaticText* m_bootLabel;
    wxStaticText* m_bootValue;
    wxStaticText* m_netLabel;
    wxStaticText* m_netValue;
    wxCheckBox* m_responseCtrl;
    wxButton* m_ok;
    wxButton* m_cancel;

    void onListItemSelected( wxCommandEvent& event ) { event.Skip(); }

public:

    CImportDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Introduction"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
    ~CImportDialog();
};
