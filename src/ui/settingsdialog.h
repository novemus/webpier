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

class CSettingsDialog : public wxDialog
{
    wxNotebook* m_notebook;
    wxTextCtrl* m_ownerCtrl;
    wxTextCtrl* m_pierCtrl;
    wxCheckBox* m_daemonCtrl;
    wxTextCtrl* m_stunCtrl;
    wxTextCtrl* m_punchCtrl;
    wxTextCtrl* m_dhtBootCtrl;
    wxTextCtrl* m_dhtPortCtrl;
    wxTextCtrl* m_smtpCtrl;
    wxTextCtrl* m_imapCtrl;
    wxTextCtrl* m_loginCtrl;
    wxTextCtrl* m_passCtrl;
    wxFilePickerCtrl* m_certPicker;
    wxFilePickerCtrl* m_keyPicker;
    wxFilePickerCtrl* m_caPicker;
    wxButton* m_cancelBtn;
    wxButton* m_okBtn;
    WebPier::Context::ConfigPtr m_config;
    bool m_daemon;

    void onOkButtonClick(wxCommandEvent& event);
    void onDaemonCheckBoxClick(wxCommandEvent& event);

public:

    CSettingsDialog( WebPier::Context::ConfigPtr config, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );
    ~CSettingsDialog();
};
