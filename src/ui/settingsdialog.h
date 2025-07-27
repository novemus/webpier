#pragma once

#include <ui/context.h>
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/filepicker.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <memory>

class CSettingsDialog : public wxDialog, public std::enable_shared_from_this<CSettingsDialog>
{
    wxNotebook* m_notebook;
    wxTextCtrl* m_ownerCtrl;
    wxTextCtrl* m_pierCtrl;
    wxCheckBox* m_daemonCtrl;
    wxPanel* m_natPanel;
    wxTextCtrl* m_stunCtrl;
    wxTextCtrl* m_punchCtrl;
    wxButton* m_stunTest;
    wxGauge* m_stunGauge;
    wxPanel* m_dhtPanel;
    wxTextCtrl* m_dhtBootCtrl;
    wxTextCtrl* m_dhtPortCtrl;
    wxButton* m_dhtTest;
    wxGauge* m_dhtGauge;
    wxPanel* m_emailPanel;
    wxTextCtrl* m_smtpCtrl;
    wxTextCtrl* m_imapCtrl;
    wxTextCtrl* m_loginCtrl;
    wxTextCtrl* m_passCtrl;
    wxFilePickerCtrl* m_certPicker;
    wxFilePickerCtrl* m_keyPicker;
    wxFilePickerCtrl* m_caPicker;
    wxButton* m_emailTest;
    wxGauge* m_emailGauge;
    wxButton* m_cancelBtn;
    wxButton* m_okBtn;
    WebPier::Context::ConfigPtr m_config;
    bool m_daemon;

    void onOkButtonClick(wxCommandEvent& event);
    void onDaemonCheckBoxClick(wxCommandEvent& event);
    void onStunTestClick(wxCommandEvent& event);
    void onDhtTestClick(wxCommandEvent& event);
    void onEmailTestClick(wxCommandEvent& event);
    void onIdle(wxIdleEvent& event);
    void onStunChange(wxCommandEvent& event);
    void onDhtChange(wxCommandEvent& event);
    void onEmailChange(wxCommandEvent& event);

public:

    CSettingsDialog( WebPier::Context::ConfigPtr config, const wxString& title = _("Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
    ~CSettingsDialog();
};
