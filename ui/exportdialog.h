#pragma once

#include "context.h"
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
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

class CExportDialog : public wxDialog
{
    wxCheckListBox* m_serviceList;
    wxStaticText* m_idLabel;
    wxStaticText* m_idValue;
    wxStaticText* m_serviceLabel;
    wxStaticText* m_serviceValue;
    wxStaticText* m_gateLabel;
    wxStaticText* m_gateValue;
    wxStaticText* m_startLabel;
    wxStaticText* m_startValue;
    wxStaticText* m_obscureLabel;
    wxStaticText* m_obscureValue;
    wxStaticText* m_bootLabel;
    wxStaticText* m_bootValue;
    wxStaticText* m_netLabel;
    wxStaticText* m_netValue;
    wxButton* m_ok;
    wxButton* m_cancel;
    wxVector<WebPier::Service> m_locals;

    void onListItemSelected( wxCommandEvent& event );
    void populate();

public:

    CExportDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Advertisement"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
    ~CExportDialog();

    wxVector<WebPier::Service> GetExport() const;
};
