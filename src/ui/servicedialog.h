#pragma once

#include "context.h"
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

class CServiceDialog : public wxDialog
{
    wxPropertyGrid* m_propGrid;
    wxPGProperty* m_nameItem;
    wxPGProperty* m_pierItem;
    wxPGProperty* m_addrItem;
    wxPGProperty* m_gateItem;
    wxPGProperty* m_obsItem;
    wxPGProperty* m_startItem;
    wxPGProperty* m_rendItem;
    wxPGProperty* m_bootItem;
    wxButton* m_ok;
    wxButton* m_cancel;
    WebPier::Context::ConfigPtr m_config;
    WebPier::Context::ServicePtr m_service;

    void onPropertyChanged( wxPropertyGridEvent& event );
    void onOKButtonClick( wxCommandEvent& event );

public:

    CServiceDialog(
        WebPier::Context::ConfigPtr config,
        WebPier::Context::ServicePtr service,
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& title = _("Service"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(-1, -1),
        long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP);

    ~CServiceDialog();
};
