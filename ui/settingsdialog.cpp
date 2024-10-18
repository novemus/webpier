#include "settingsdialog.h"
#include "messagedialog.h"
#include <wx/msgdlg.h> 
#include <wx/valnum.h>

namespace
{
   static constexpr const char* FORBIDDEN_PATH_CHARS = "*/\\<>:|?";
}

CSettingsDialog::CSettingsDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style)
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    mainSizer->SetMinSize( wxSize( 450,-1 ) );
    m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_FIXEDWIDTH );
    wxPanel* basicPanel;
    basicPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    basicPanel->SetToolTip( _("Basic settings") );

    wxBoxSizer* basicSizer;
    basicSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBoxSizer* idSizer;
    idSizer = new wxStaticBoxSizer( new wxStaticBox( basicPanel, wxID_ANY, _("Identity") ), wxVERTICAL );

    wxFlexGridSizer* idGridSizer;
    idGridSizer = new wxFlexGridSizer( 0, 2, 5, 5 );
    idGridSizer->AddGrowableCol( 1 );
    idGridSizer->SetFlexibleDirection( wxBOTH );
    idGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText* ownerLabel;
    ownerLabel = new wxStaticText( idSizer->GetStaticBox(), wxID_ANY, _("Owner"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    ownerLabel->Wrap( -1 );
    idGridSizer->Add( ownerLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_ownerCtrl = new wxTextCtrl( idSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_ownerCtrl->SetToolTip( _("Email address to represent you to peers and use it for the email rendezvous") );
    m_ownerCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator*)m_ownerCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add( m_ownerCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* pierLabel;
    pierLabel = new wxStaticText( idSizer->GetStaticBox(), wxID_ANY, _("Pier"), wxDefaultPosition, wxDefaultSize, 0 );
    pierLabel->Wrap( -1 );
    idGridSizer->Add( pierLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_pierCtrl = new wxTextCtrl( idSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_pierCtrl->SetToolTip( _("Identifier of the pier") );
    m_pierCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator*)m_pierCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add( m_pierCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    idSizer->Add( idGridSizer, 1, wxALL|wxEXPAND, 5 );

    basicSizer->Add( idSizer, 0, wxEXPAND|wxALL, 10 );

    m_startupCheckBox = new wxCheckBox( basicPanel, wxID_ANY, _("Launch daemon on system startup"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startupCheckBox->SetValue(true);
    basicSizer->Add( m_startupCheckBox, 0, wxALL, 5 );

    m_trayCheckBox = new wxCheckBox( basicPanel, wxID_ANY, _("Show tray icon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_trayCheckBox->SetValue(true);
    basicSizer->Add( m_trayCheckBox, 0, wxALL, 5 );

    wxBoxSizer* logSizer;
    logSizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticText* logLabel;
    logLabel = new wxStaticText( basicPanel, wxID_ANY, _("Logging"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    logLabel->Wrap( -1 );
    logSizer->Add( logLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    wxString m_logChoiceChoices[] = { _("fatal"), _("error"), _("warning"), _("info"), _("debug"), _("trace") };
    int m_logChoiceNChoices = sizeof( m_logChoiceChoices ) / sizeof( wxString );
    m_logChoice = new wxChoice( basicPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_logChoiceNChoices, m_logChoiceChoices, 0 );
    m_logChoice->SetSelection( 3 );
    logSizer->Add( m_logChoice, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    basicSizer->Add( logSizer, 0, wxTOP|wxRIGHT|wxLEFT|wxEXPAND, 5 );

    basicPanel->SetSizer( basicSizer );
    basicPanel->Layout();
    basicSizer->Fit( basicPanel );
    m_notebook->AddPage( basicPanel, _("Basic"), true );
    wxPanel* natPanel;
    natPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    natPanel->SetToolTip( _("NAT traverse settings") );

    wxBoxSizer* stunSizer;
    stunSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* stunGridSizer;
    stunGridSizer = new wxFlexGridSizer( 0, 2, 5, 5 );
    stunGridSizer->AddGrowableCol( 1 );
    stunGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    stunGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText* stunLabel;
    stunLabel = new wxStaticText( natPanel, wxID_ANY, _("STUN server"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    stunLabel->Wrap( -1 );
    stunGridSizer->Add( stunLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_stunCtrl = new wxTextCtrl( natPanel, wxID_ANY, _("stun.ekiga.net"), wxDefaultPosition, wxDefaultSize, 0 );
    stunGridSizer->Add( m_stunCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

    wxStaticText* punchLabel;
    punchLabel = new wxStaticText( natPanel, wxID_ANY, _("Punch hops"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    punchLabel->Wrap( -1 );
    stunGridSizer->Add( punchLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_punchCtrl = new wxTextCtrl( natPanel, wxID_ANY, _("7"), wxDefaultPosition, wxDefaultSize, 0 );
    m_punchCtrl->SetValidator( wxIntegerValidator<unsigned char>() );

    stunGridSizer->Add( m_punchCtrl, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

    stunSizer->Add( stunGridSizer, 0, wxEXPAND|wxALL, 5 );

    natPanel->SetSizer( stunSizer );
    natPanel->Layout();
    stunSizer->Fit( natPanel );
    m_notebook->AddPage( natPanel, _("NAT"), false );
    wxPanel* dhtPanel;
    dhtPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    dhtPanel->SetToolTip( _("DHT rendezvous settings") );

    wxBoxSizer* dhtSizer;
    dhtSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* dhtGridSizer;
    dhtGridSizer = new wxFlexGridSizer( 0, 2, 5, 5 );
    dhtGridSizer->AddGrowableCol( 1 );
    dhtGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    dhtGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText* dhtBootLabel;
    dhtBootLabel = new wxStaticText( dhtPanel, wxID_ANY, _("Bootstrap"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    dhtBootLabel->Wrap( -1 );
    dhtGridSizer->Add( dhtBootLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_dhtBootCtrl = new wxTextCtrl( dhtPanel, wxID_ANY, _("bootstrap.jami.net:4222"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    dhtGridSizer->Add( m_dhtBootCtrl, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

    wxStaticText* dhtNetLabel;
    dhtNetLabel = new wxStaticText( dhtPanel, wxID_ANY, _("Network"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    dhtNetLabel->Wrap( -1 );
    dhtGridSizer->Add( dhtNetLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_dhtNetCtrl = new wxTextCtrl( dhtPanel, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
    m_dhtNetCtrl->SetValidator( wxIntegerValidator<unsigned int>() );
    dhtGridSizer->Add( m_dhtNetCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    dhtSizer->Add( dhtGridSizer, 0, wxALL|wxEXPAND, 5 );

    dhtPanel->SetSizer( dhtSizer );
    dhtPanel->Layout();
    dhtSizer->Fit( dhtPanel );
    m_notebook->AddPage( dhtPanel, _("DHT"), false );
    wxPanel* emailPanel;
    emailPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    emailPanel->SetToolTip( _("Email settings") );

    wxBoxSizer* emailSizer;
    emailSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* emailGridSizer;
    emailGridSizer = new wxFlexGridSizer( 0, 2, 5, 5 );
    emailGridSizer->AddGrowableCol( 1 );
    emailGridSizer->SetFlexibleDirection( wxBOTH );
    emailGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText* smtpLabel;
    smtpLabel = new wxStaticText( emailPanel, wxID_ANY, _("SMTP"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    smtpLabel->Wrap( -1 );
    emailGridSizer->Add( smtpLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_smtpCtrl = new wxTextCtrl( emailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    emailGridSizer->Add( m_smtpCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    wxStaticText* imapLabel;
    imapLabel = new wxStaticText( emailPanel, wxID_ANY, _("IMAP"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    imapLabel->Wrap( -1 );
    emailGridSizer->Add( imapLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_imapCtrl = new wxTextCtrl( emailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    emailGridSizer->Add( m_imapCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* loginLabel;
    loginLabel = new wxStaticText( emailPanel, wxID_ANY, _("Login"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    loginLabel->Wrap( -1 );
    emailGridSizer->Add( loginLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_loginCtrl = new wxTextCtrl( emailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    emailGridSizer->Add( m_loginCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* passLabel;
    passLabel = new wxStaticText( emailPanel, wxID_ANY, _("Password"), wxDefaultPosition, wxDefaultSize, 0 );
    passLabel->Wrap( -1 );
    passLabel->SetMinSize( wxSize( 100,-1 ) );

    emailGridSizer->Add( passLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_passCtrl = new wxTextCtrl( emailPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    emailGridSizer->Add( m_passCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* certLabel;
    certLabel = new wxStaticText( emailPanel, wxID_ANY, _("Certificate"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    certLabel->Wrap( -1 );
    emailGridSizer->Add( certLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_certPicker = new wxFilePickerCtrl( emailPanel, wxID_ANY, wxEmptyString, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
    emailGridSizer->Add( m_certPicker, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* keyLabel;
    keyLabel = new wxStaticText( emailPanel, wxID_ANY, _("Key"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    keyLabel->Wrap( -1 );
    emailGridSizer->Add( keyLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_keyPicker = new wxFilePickerCtrl( emailPanel, wxID_ANY, wxEmptyString, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
    emailGridSizer->Add( m_keyPicker, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* caLabel;
    caLabel = new wxStaticText( emailPanel, wxID_ANY, _("CA"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    caLabel->Wrap( -1 );
    emailGridSizer->Add( caLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_caPicker = new wxFilePickerCtrl( emailPanel, wxID_ANY, wxEmptyString, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
    emailGridSizer->Add( m_caPicker, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    emailSizer->Add( emailGridSizer, 1, wxEXPAND|wxALL, 5 );

    emailPanel->SetSizer( emailSizer );
    emailPanel->Layout();
    emailSizer->Fit( emailPanel );
    m_notebook->AddPage( emailPanel, _("Email"), false );

    mainSizer->Add( m_notebook, 1, wxEXPAND | wxALL, 5 );

    wxBoxSizer* footSizer;
    footSizer = new wxBoxSizer( wxHORIZONTAL );

    footSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    m_cancelBtn = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );

    m_cancelBtn->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON) ) );
    footSizer->Add( m_cancelBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_okBtn = new wxButton( this, wxID_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );

    m_okBtn->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON) ) );
    footSizer->Add( m_okBtn, 0, wxALL, 5 );

    mainSizer->Add( footSizer, 0, wxEXPAND, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    m_cancelBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CSettingsDialog::onCancelButtonClick ), NULL, this );
    m_okBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CSettingsDialog::onOkButtonClick ), NULL, this );
}

CSettingsDialog::~CSettingsDialog()
{
    delete m_notebook;
    delete m_cancelBtn;
    delete m_okBtn;
}

void CSettingsDialog::onOkButtonClick(wxCommandEvent& event)
{
    auto owner = m_ownerCtrl->GetValue();
    auto pier = m_pierCtrl->GetValue();
    auto stun = m_stunCtrl->GetValue();
    auto dht = m_dhtBootCtrl->GetValue();
    auto smtp = m_smtpCtrl->GetValue();
    auto imap = m_imapCtrl->GetValue();
    auto login = m_loginCtrl->GetValue();
    auto passwd = m_passCtrl->GetValue();

    if (owner.IsEmpty() || pier.IsEmpty() || stun.IsEmpty() || (dht.IsEmpty() && (smtp.IsEmpty() || imap.IsEmpty() || login.IsEmpty() || passwd.IsEmpty())))
    {
        CMessageDialog dialog(this, _("To use WebPier you must define identity, STUN and either DHT or Email rendezvous"), wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
        dialog.ShowModal();
        return;
    }

    event.Skip();
}
