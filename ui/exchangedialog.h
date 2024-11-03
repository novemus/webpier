#pragma once

#include "context.h"
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/string.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/checklst.h>
#include <wx/panel.h>

class CImportPage : public wxPanel
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

    wxVector<WebPier::ServicePtr> m_remotes;

    void onListItemSelected( wxCommandEvent& event );
    void onServiceCtrlKillFocus( wxFocusEvent& event );
	void onGatewayCtrlKillFocus( wxFocusEvent& event );
	void onAutostartCheckBox( wxCommandEvent& event );
    void populate(int line);

public:

    CImportPage(const wxString& pier, const WebPier::ServiceList& remotes, wxWindow* parent);
    ~CImportPage();

    bool ValidateData();
    WebPier::ServiceList GetImport() const;
};

class CExportPage : public wxPanel
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

    wxString m_pier;
    wxVector<WebPier::ServicePtr> m_locals;

    void onListItemSelected( wxCommandEvent& event );
    void onListItemToggled( wxCommandEvent& event );
    void populate(int line);

public:

    CExportPage(const wxString& pier, const WebPier::ServiceList& locals, wxWindow* parent);
    ~CExportPage();

    WebPier::ServiceList GetExport() const;
};

class CExchangeDialog : public wxDialog
{
    CImportPage* m_importPage;
    CExportPage* m_exportPage;
    wxCheckBox* m_purge;
    wxCheckBox* m_reply;
    wxButton* m_back;
    wxButton* m_next;
    wxButton* m_cancel;

    void onBackButtonClick(wxCommandEvent& event);
    void onNextButtonClick(wxCommandEvent& event);

public:

    CExchangeDialog(const wxString& pier, const WebPier::ServiceList& remotes, const WebPier::ServiceList& locals, wxWindow* parent);
    CExchangeDialog(const wxString& pier, const WebPier::ServiceList& locals, wxWindow* parent);

    ~CExchangeDialog();

    bool IsNeedImportMerge() const { return !m_purge->IsChecked(); }
    bool IsNeedExportReply() const { return m_reply->IsChecked(); }
    WebPier::ServiceList GetImport() const { return m_importPage->GetImport(); }
    WebPier::ServiceList GetExport() const { return m_exportPage->GetExport(); }
};
