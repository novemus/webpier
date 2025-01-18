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
    wxStaticText* m_nameLabel;
    wxStaticText* m_nameValue;
    wxStaticText* m_addressLabel;
    wxTextCtrl* m_addressCtrl;
    wxStaticText* m_startLabel;
    wxCheckBox* m_startCtrl;
    wxStaticText* m_obscureLabel;
    wxStaticText* m_obscureValue;
    wxStaticText* m_rendLabel;
    wxStaticText* m_rendValue;

    wxVector<WebPier::Context::ServicePtr> m_import;

    void onListItemSelected( wxCommandEvent& event );
    void onServiceCtrlKillFocus( wxFocusEvent& event );
	void onAutostartCheckBox( wxCommandEvent& event );
    void populate(int line);

public:

    CImportPage(const wxString& pier, const WebPier::Context::ServiceList& forImport, wxWindow* parent);
    ~CImportPage();

    bool ValidateData();
    WebPier::Context::ServiceList GetImport() const;
};

class CExportPage : public wxPanel
{
    wxCheckListBox* m_serviceList;
    wxStaticText* m_nameLabel;
    wxStaticText* m_nameValue;
    wxStaticText* m_addressLabel;
    wxStaticText* m_addressValue;
    wxStaticText* m_startLabel;
    wxStaticText* m_startValue;
    wxStaticText* m_obscureLabel;
    wxStaticText* m_obscureValue;
    wxStaticText* m_rendLabel;
    wxStaticText* m_rendValue;

    wxVector<WebPier::Context::ServicePtr> m_export;

    void onListItemSelected( wxCommandEvent& event );
    void populate(int line);

public:

    CExportPage(const wxString& pier, const WebPier::Context::ServiceList& forExport, wxWindow* parent);
    ~CExportPage();

    WebPier::Context::ServiceList GetExport() const;
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

    CExchangeDialog(const wxString& pier, const WebPier::Context::ServiceList& forImport, const WebPier::Context::ServiceList& forExport, wxWindow* parent);
    CExchangeDialog(const wxString& pier, const WebPier::Context::ServiceList& forExport, wxWindow* parent);

    ~CExchangeDialog();

    bool NeedImportMerge() const { return !m_purge->IsChecked(); }
    bool NeedExportReply() const { return m_reply->IsChecked(); }
    WebPier::Context::ServiceList GetImport() const { return m_importPage->GetImport(); }
    WebPier::Context::ServiceList GetExport() const { return m_exportPage->GetExport(); }
};
