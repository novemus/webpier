#include <ui/settingsdialog.h>
#include <ui/messagedialog.h>
#include <ui/logo.h>
#include <wx/msgdlg.h>
#include <wx/valnum.h>
#include <wx/datetime.h>
#include <wx/menu.h>
#include <wx/notifmsg.h>
#include <wx/regex.h>
#include <cstdint>

#ifdef WIN32
#include <Shlobj_core.h>
#endif

CSettingsDialog::CSettingsDialog(WebPier::Context::ConfigPtr config, const wxString &title, const wxPoint &pos, const wxSize &size, long style)
    : wxDialog(nullptr, wxID_ANY, title, pos, size, style), m_config(config)
{
    static constexpr const char *FORBIDDEN_PATH_CHARS = "*/\\<>:|?\" \t\n\r";

    this->SetIcon(::GetAppIconBundle().GetIcon());
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer *mainSizer;
    mainSizer = new wxBoxSizer(wxVERTICAL);

    mainSizer->SetMinSize(wxSize(450, -1));
    m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_FIXEDWIDTH);
    wxPanel *basicPanel;
    basicPanel = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    basicPanel->SetToolTip(_("Basic settings"));

    wxBoxSizer *basicSizer;
    basicSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer *idSizer;
    idSizer = new wxStaticBoxSizer(new wxStaticBox(basicPanel, wxID_ANY, _("Identity")), wxVERTICAL);

    wxFlexGridSizer *idGridSizer;
    idGridSizer = new wxFlexGridSizer(0, 2, 5, 5);
    idGridSizer->AddGrowableCol(1);
    idGridSizer->SetFlexibleDirection(wxBOTH);
    idGridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxStaticText *ownerLabel;
    ownerLabel = new wxStaticText(idSizer->GetStaticBox(), wxID_ANY, _("Owner"), wxDefaultPosition, wxSize(100, -1), 0);
    ownerLabel->Wrap(-1);
    idGridSizer->Add(ownerLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_ownerCtrl = new wxTextCtrl(idSizer->GetStaticBox(), wxID_ANY, m_config->Pier.Before('/'), wxDefaultPosition, wxDefaultSize, 0);
    m_ownerCtrl->SetToolTip(_("The email address that represents you to owners of remote piers"));
    m_ownerCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator *)m_ownerCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add(m_ownerCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    wxStaticText *pierLabel;
    pierLabel = new wxStaticText(idSizer->GetStaticBox(), wxID_ANY, _("Pier"), wxDefaultPosition, wxDefaultSize, 0);
    pierLabel->Wrap(-1);
    idGridSizer->Add(pierLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_pierCtrl = new wxTextCtrl(idSizer->GetStaticBox(), wxID_ANY, m_config->Pier.After('/'), wxDefaultPosition, wxDefaultSize, 0);
    m_pierCtrl->SetToolTip(_("The identifier of this pier"));
    m_pierCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator *)m_pierCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add(m_pierCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);
    idSizer->Add(idGridSizer, 1, wxALL | wxEXPAND, 5);
    basicSizer->Add(idSizer, 0, wxEXPAND | wxALL, 10);

    m_daemonCtrl = new wxCheckBox(basicPanel, wxID_ANY, _("Run the backend at system startup"), wxDefaultPosition, wxDefaultSize, 0);
    if (WebPier::Backend::CouldAutostart())
    {
        m_daemon = WebPier::Backend::VerifyAutostart();
    }
    else
    {
        m_daemon = false;
        m_daemonCtrl->Enable(false);
#ifndef WIN32
        m_daemonCtrl->SetToolTip(_("This option depends on the 'crontab' tool"));
#else
        m_daemonCtrl->SetToolTip(_("This option depends on the 'schtasks' tool"));
#endif
    }
    m_daemonCtrl->SetValue(m_daemon);

    basicSizer->Add(m_daemonCtrl, 0, wxALL, 5);

    basicPanel->SetSizer(basicSizer);
    basicPanel->Layout();
    basicSizer->Fit(basicPanel);
    m_notebook->AddPage(basicPanel, _("Basic"), true);
    m_natPanel = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_natPanel->SetToolTip(_("NAT traverse settings"));

    wxBoxSizer *stunSizer;
    stunSizer = new wxBoxSizer(wxVERTICAL);

    stunSizer->Add( 0, 10, 0, wxEXPAND, 5 );

    m_isNat = new wxCheckBox(m_natPanel, wxID_ANY, _("Make traverse"), wxDefaultPosition, wxDefaultSize, 0);
    m_isNat->SetValue(m_config->NatTest != WebPier::Context::Config::Checkup::Noneed);
    stunSizer->Add(m_isNat, 0, wxRIGHT|wxLEFT, 10);

    stunSizer->Add( 0, 5, 0, wxEXPAND, 5 );

    wxFlexGridSizer *stunGridSizer;
    stunGridSizer = new wxFlexGridSizer(0, 2, 5, 5);
    stunGridSizer->AddGrowableCol(1);
    stunGridSizer->AddGrowableRow(4);
    stunGridSizer->SetFlexibleDirection(wxBOTH);
    stunGridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxStaticText *testLabel = new wxStaticText(m_natPanel, wxID_ANY, _("Checkup"), wxDefaultPosition, wxDefaultSize, 0);
    testLabel->Wrap(-1);
    stunGridSizer->Add(testLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxString testModeChoices[] = {_("Strict"), _("Faulty"), _("Simple")};
    m_testMode = new wxChoice(m_natPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, sizeof(testModeChoices) / sizeof(wxString), testModeChoices, 0);
    m_testMode->SetSelection(m_config->NatTest == WebPier::Context::Config::Checkup::Noneed ? 0 : m_config->NatTest - 1);
    stunGridSizer->Add(m_testMode, 0, wxALL | wxEXPAND, 5);

    wxStaticText *hopsLabel = new wxStaticText(m_natPanel, wxID_ANY, _("NAT hops"), wxDefaultPosition, wxSize(100, -1), 0);
    hopsLabel->Wrap(-1);
    stunGridSizer->Add(hopsLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_hopsCtrl = new wxTextCtrl(m_natPanel, wxID_ANY, wxString::Format(wxT("%d"), (int)m_config->NatHops), wxDefaultPosition, wxDefaultSize, 0);
    stunGridSizer->Add(m_hopsCtrl, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    wxStaticText *udpStunLabel = new wxStaticText(m_natPanel, wxID_ANY, _("UDP STUN"), wxDefaultPosition, wxSize(100, -1), 0);
    udpStunLabel->Wrap(-1);
    stunGridSizer->Add(udpStunLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_udpStunCtrl = new wxTextCtrl(m_natPanel, wxID_ANY, m_config->UdpStunServer, wxDefaultPosition, wxDefaultSize, 0);
    stunGridSizer->Add(m_udpStunCtrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    wxStaticText *tcpStunLabel = new wxStaticText(m_natPanel, wxID_ANY, _("TCP STUN"), wxDefaultPosition, wxSize(100, -1), 0);
    tcpStunLabel->Wrap(-1);
    stunGridSizer->Add(tcpStunLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_tcpStunCtrl = new wxTextCtrl(m_natPanel, wxID_ANY, m_config->TcpStunServer, wxDefaultPosition, wxDefaultSize, 0);
    stunGridSizer->Add(m_tcpStunCtrl, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_udpStunTest = new wxButton(m_natPanel, wxID_ANY, _("Test UDP"), wxDefaultPosition, wxDefaultSize, 0);

    m_udpStunTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    stunGridSizer->Add(m_udpStunTest, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);

    m_udpGauge = new wxGauge(m_natPanel, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
    stunGridSizer->Add(m_udpGauge, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5);

    m_tcpStunTest = new wxButton(m_natPanel, wxID_ANY, _("Test TCP"), wxDefaultPosition, wxDefaultSize, 0);

    m_tcpStunTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    stunGridSizer->Add(m_tcpStunTest, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT | wxEXPAND, 5);

    m_tcpGauge = new wxGauge(m_natPanel, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
    stunGridSizer->Add(m_tcpGauge, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5);

    stunSizer->Add(stunGridSizer, 0, wxEXPAND | wxALL, 5);

    m_natPanel->SetSizer(stunSizer);
    m_natPanel->Layout();
    stunSizer->Fit(m_natPanel);
    m_notebook->AddPage(m_natPanel, _("NAT"), false);

    auto val = m_isNat->GetValue();
    m_testMode->Enable(val);
    m_hopsCtrl->Enable(val);
    m_udpStunCtrl->Enable(val);
    m_tcpStunCtrl->Enable(val);
    m_udpStunTest->Enable(val);
    m_tcpStunTest->Enable(val);

    m_tcpGauge->Hide();
    m_udpGauge->Hide();

    m_dhtPanel = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_dhtPanel->SetToolTip(_("DHT node settings"));

    wxBoxSizer *dhtSizer;
    dhtSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer *dhtGridSizer;
    dhtGridSizer = new wxFlexGridSizer(0, 2, 5, 5);
    dhtGridSizer->AddGrowableCol(1);
    dhtGridSizer->SetFlexibleDirection(wxHORIZONTAL);
    dhtGridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxStaticText *dhtBootLabel;
    dhtBootLabel = new wxStaticText(m_dhtPanel, wxID_ANY, _("Bootstrap"), wxDefaultPosition, wxSize(100, -1), 0);
    dhtBootLabel->Wrap(-1);
    dhtGridSizer->Add(dhtBootLabel, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxRIGHT | wxLEFT, 5);

    m_dhtBootCtrl = new wxTextCtrl(m_dhtPanel, wxID_ANY, m_config->DhtBootstrap, wxDefaultPosition, wxSize(-1, -1), 0);
    dhtGridSizer->Add(m_dhtBootCtrl, 0, wxEXPAND | wxTOP | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5);

    wxStaticText *dhtPortLabel;
    dhtPortLabel = new wxStaticText(m_dhtPanel, wxID_ANY, _("Port"), wxDefaultPosition, wxSize(100, -1), 0);
    dhtPortLabel->Wrap(-1);
    dhtGridSizer->Add(dhtPortLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_dhtPortCtrl = new wxTextCtrl(m_dhtPanel, wxID_ANY, wxString::Format(wxT("%d"), (int)m_config->DhtPort), wxDefaultPosition, wxDefaultSize, 0);
    m_dhtPortCtrl->SetValidator(wxIntegerValidator<unsigned int>());
    dhtGridSizer->Add(m_dhtPortCtrl, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_dhtTest = new wxButton(m_dhtPanel, wxID_ANY, _("Test"), wxDefaultPosition, wxDefaultSize, 0);
    m_dhtTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    dhtGridSizer->Add(m_dhtTest, 0, wxALIGN_CENTER_VERTICAL, 10);

    m_dhtGauge = new wxGauge(m_dhtPanel, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
    m_dhtGauge->Hide();

    dhtGridSizer->Add(m_dhtGauge, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    dhtSizer->Add(dhtGridSizer, 0, wxALL | wxEXPAND, 5);

    m_dhtPanel->SetSizer(dhtSizer);
    m_dhtPanel->Layout();
    dhtSizer->Fit(m_dhtPanel);
    m_notebook->AddPage(m_dhtPanel, _("DHT"), false);

    m_emailPanel = new wxPanel(m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_emailPanel->SetToolTip(_("Email settings"));

    wxBoxSizer *emailSizer;
    emailSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer *emailGridSizer;
    emailGridSizer = new wxFlexGridSizer(0, 2, 5, 5);
    emailGridSizer->AddGrowableCol(1);
    emailGridSizer->SetFlexibleDirection(wxBOTH);
    emailGridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxStaticText *smtpLabel;
    smtpLabel = new wxStaticText(m_emailPanel, wxID_ANY, _("SMTP"), wxDefaultPosition, wxSize(100, -1), 0);
    smtpLabel->Wrap(-1);
    emailGridSizer->Add(smtpLabel, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxRIGHT | wxLEFT, 5);

    m_smtpCtrl = new wxTextCtrl(m_emailPanel, wxID_ANY, m_config->SmtpServer, wxDefaultPosition, wxDefaultSize, 0);
    emailGridSizer->Add(m_smtpCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxTOP | wxRIGHT | wxLEFT, 5);

    wxStaticText *imapLabel;
    imapLabel = new wxStaticText(m_emailPanel, wxID_ANY, _("IMAP"), wxDefaultPosition, wxSize(100, -1), 0);
    imapLabel->Wrap(-1);
    emailGridSizer->Add(imapLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_imapCtrl = new wxTextCtrl(m_emailPanel, wxID_ANY, m_config->ImapServer, wxDefaultPosition, wxDefaultSize, 0);
    emailGridSizer->Add(m_imapCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    wxStaticText *loginLabel;
    loginLabel = new wxStaticText(m_emailPanel, wxID_ANY, _("Login"), wxDefaultPosition, wxSize(100, -1), 0);
    loginLabel->Wrap(-1);
    emailGridSizer->Add(loginLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_loginCtrl = new wxTextCtrl(m_emailPanel, wxID_ANY, m_config->EmailLogin, wxDefaultPosition, wxDefaultSize, 0);
    emailGridSizer->Add(m_loginCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    wxStaticText *passLabel;
    passLabel = new wxStaticText(m_emailPanel, wxID_ANY, _("Password"), wxDefaultPosition, wxDefaultSize, 0);
    passLabel->Wrap(-1);
    passLabel->SetMinSize(wxSize(100, -1));

    emailGridSizer->Add(passLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_passCtrl = new wxTextCtrl(m_emailPanel, wxID_ANY, m_config->EmailPassword, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    emailGridSizer->Add(m_passCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    wxStaticText *certLabel;
    certLabel = new wxStaticText(m_emailPanel, wxID_ANY, _("Certificate"), wxDefaultPosition, wxSize(100, -1), 0);
    certLabel->Wrap(-1);
    emailGridSizer->Add(certLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_certPicker = new wxFilePickerCtrl(m_emailPanel, wxID_ANY, m_config->EmailX509Cert, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE);
    emailGridSizer->Add(m_certPicker, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    wxStaticText *keyLabel;
    keyLabel = new wxStaticText(m_emailPanel, wxID_ANY, _("Key"), wxDefaultPosition, wxSize(100, -1), 0);
    keyLabel->Wrap(-1);
    emailGridSizer->Add(keyLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_keyPicker = new wxFilePickerCtrl(m_emailPanel, wxID_ANY, m_config->EmailX509Key, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE);
    emailGridSizer->Add(m_keyPicker, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    wxStaticText *caLabel;
    caLabel = new wxStaticText(m_emailPanel, wxID_ANY, _("CA"), wxDefaultPosition, wxSize(100, -1), 0);
    caLabel->Wrap(-1);
    emailGridSizer->Add(caLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_caPicker = new wxFilePickerCtrl(m_emailPanel, wxID_ANY, m_config->EmailX509Ca, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE);
    emailGridSizer->Add(m_caPicker, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_emailTest = new wxButton(m_emailPanel, wxID_ANY, _("Test"), wxDefaultPosition, wxDefaultSize, 0);
    m_emailTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    emailGridSizer->Add(m_emailTest, 0, wxALIGN_CENTER_VERTICAL, 10);

    m_emailGauge = new wxGauge(m_emailPanel, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
    m_emailGauge->Hide();

    emailGridSizer->Add(m_emailGauge, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    emailSizer->Add(emailGridSizer, 1, wxEXPAND | wxALL, 5);

    m_emailPanel->SetSizer(emailSizer);
    m_emailPanel->Layout();
    emailSizer->Fit(m_emailPanel);
    m_notebook->AddPage(m_emailPanel, _("Email"), false);

    mainSizer->Add(m_notebook, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer *footSizer;
    footSizer = new wxBoxSizer(wxHORIZONTAL);

    footSizer->Add(0, 0, 1, wxEXPAND, 5);
    m_cancelBtn = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0);

    footSizer->Add(m_cancelBtn, 0, wxTOP | wxBOTTOM | wxLEFT, 5);
    m_okBtn = new wxButton(this, wxID_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0);

#ifndef WIN32
    m_cancelBtn->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON)));
    m_okBtn->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON)));
#endif

    footSizer->Add(m_okBtn, 0, wxALL, 5);

    mainSizer->Add(footSizer, 0, wxEXPAND, 5);

    this->SetSizer(mainSizer);
    this->Layout();
    mainSizer->Fit(this);

    this->Centre(wxBOTH);

    this->Connect(wxEVT_IDLE, wxIdleEventHandler(CSettingsDialog::onIdle));
    m_isNat->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CSettingsDialog::onNatCheckBoxClick ), NULL, this );
    m_okBtn->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::onOkButtonClick), NULL, this);
    m_udpStunCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onUdpStunChange), NULL, this);
    m_udpStunTest->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::onUdpStunTestClick), NULL, this);
    m_tcpStunCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onTcpStunChange), NULL, this);
    m_tcpStunTest->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::onTcpStunTestClick), NULL, this);
    m_dhtBootCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onDhtChange), NULL, this);
    m_dhtPortCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onDhtChange), NULL, this);
    m_dhtTest->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::onDhtTestClick), NULL, this);
    m_smtpCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onEmailChange), NULL, this);
    m_imapCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onEmailChange), NULL, this);
    m_loginCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onEmailChange), NULL, this);
    m_passCtrl->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onEmailChange), NULL, this);
    m_certPicker->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onEmailChange), NULL, this);
    m_keyPicker->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onEmailChange), NULL, this);
    m_caPicker->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(CSettingsDialog::onEmailChange), NULL, this);
    m_emailTest->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CSettingsDialog::onEmailTestClick), NULL, this);
}

CSettingsDialog::~CSettingsDialog()
{
    delete m_notebook;
    delete m_cancelBtn;
    delete m_okBtn;
}

void CSettingsDialog::onIdle(wxIdleEvent &event)
{
    if (m_udpGauge->IsShown())
        m_udpGauge->Pulse();

    if (m_tcpGauge->IsShown())
        m_tcpGauge->Pulse();

    if (m_dhtGauge->IsShown())
        m_dhtGauge->Pulse();

    if (m_emailGauge->IsShown())
        m_emailGauge->Pulse();

    event.Skip();
}

void CSettingsDialog::onNatCheckBoxClick(wxCommandEvent& event)
{
    auto val = m_isNat->GetValue();
    m_testMode->Enable(val);
    m_hopsCtrl->Enable(val);
    m_udpStunCtrl->Enable(val);
    m_tcpStunCtrl->Enable(val);
    m_udpStunTest->Enable(val);
    m_tcpStunTest->Enable(val);
    event.Skip();
}

void CSettingsDialog::onUdpStunChange(wxCommandEvent &event)
{
    m_udpStunTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    m_natPanel->SetToolTip(wxEmptyString);
    event.Skip();
}

void CSettingsDialog::onTcpStunChange(wxCommandEvent &event)
{
    m_tcpStunTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    m_natPanel->SetToolTip(wxEmptyString);
    event.Skip();
}

void CSettingsDialog::onDhtChange(wxCommandEvent &event)
{
    m_dhtTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    m_dhtPanel->SetToolTip(wxEmptyString);
    event.Skip();
}

void CSettingsDialog::onEmailChange(wxCommandEvent &event)
{
    m_emailTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    m_emailPanel->SetToolTip(wxEmptyString);
    event.Skip();
}

void CSettingsDialog::doExploreNat(WebPier::Context::Service::Protocol proto, wxString bind)
{
    wxString stun = proto == WebPier::Context::Service::UDP ? m_udpStunCtrl->GetValue() : m_tcpStunCtrl->GetValue();
    wxButton *button = proto == WebPier::Context::Service::UDP ? m_udpStunTest : m_tcpStunTest;
    wxGauge* gauge = proto == WebPier::Context::Service::UDP ? m_udpGauge : m_tcpGauge;
    WebPier::Context::Config::Checkup mode = m_isNat->GetValue() 
        ? static_cast<WebPier::Context::Config::Checkup>(m_testMode->GetSelection() + 1) 
        : WebPier::Context::Config::Checkup::Noneed;

    std::weak_ptr<CSettingsDialog> weak = shared_from_this();
    WebPier::Utils::ExploreNat(proto, bind, stun, mode, [this, weak, proto, button, gauge](const WebPier::Utils::Traverse &pass, const wxString &error)
    {
        if(auto ptr = weak.lock())
        {
            ptr->CallAfter([this, ptr, proto, button, gauge, pass, error]()
            {
                button->Enable();
                gauge->SetValue(0);
                gauge->Hide();

                wxString label = proto == WebPier::Context::Service::UDP ? wxT("UDP") : wxT("TCP");
                if (error.IsEmpty())
                {
                    auto& hole = proto == WebPier::Context::Service::UDP ? pass.Udp : pass.Tcp;
                    button->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(hole.Mapping == WebPier::Utils::Traverse::Independent ? wxART_TICK_MARK : wxART_WARNING), wxASCII_STR(wxART_BUTTON)));
                    m_natPanel->SetToolTip(wxString::Format(_("%s\nNAT: %s\nHairpin: %s\nRandom port: %s\nVariable IP: %s\nMapping: %s\nFiltering: %s\nInner EP: %s\nMapped EP: %s"),
                            label,
                            ToString(hole.Nat),
                            ToString(hole.Hairpin),
                            ToString(hole.RandomPort),
                            ToString(hole.VariableAddress),
                            ToString(hole.Mapping),
                            ToString(hole.Filtering),
                            hole.InnerEndpoint,
                            hole.OuterEndpoint
                        ));
                }
                else
                {
                    button->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON)));
                    m_natPanel->SetToolTip(wxString::Format(_("%s\n%s"), label, error));
                }
            });
        }
    });

    gauge->Show();
    gauge->Pulse();
    m_natPanel->Layout();
    button->Disable();
}

void CSettingsDialog::onUdpStunTestClick(wxCommandEvent &event)
{
    if (m_udpStunCtrl->GetValue().IsEmpty())
    {
        m_udpStunTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON)));
        m_natPanel->SetToolTip(_("No UDP STUN server specified!"));
        return;
    }

    bool isIPv4 = wxRegEx("^((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))(?::\\d+)?$").Matches(m_udpStunCtrl->GetValue());
    bool isIPv6 = wxRegEx("^(\\[([a-fA-F0-9:]+)\\](?::\\d+)?|(?<!\\[)([a-fA-F0-9:]+)(?!\\]))$").Matches(m_udpStunCtrl->GetValue());

    if (!isIPv4 && !isIPv6)
    {
        wxMenu *menu = new wxMenu();
        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent &)
                   { doExploreNat(WebPier::Context::Service::UDP, wxT("0.0.0.0:0")); }, menu->Append(wxID_ANY, _("IPv&4"))->GetId());

        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent &)
                   { doExploreNat(WebPier::Context::Service::UDP, wxT("[::]:0")); }, menu->Append(wxID_ANY, _("IPv&6"))->GetId());

        PopupMenu(menu);
    }
    else
    {
        doExploreNat(WebPier::Context::Service::UDP, isIPv4 ? wxT("0.0.0.0:0") : wxT("[::]:0"));
    }
}

void CSettingsDialog::onTcpStunTestClick(wxCommandEvent &event)
{
    if (m_tcpStunCtrl->GetValue().IsEmpty())
    {
        m_tcpStunTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON)));
        m_natPanel->SetToolTip(_("No TCP STUN server specified!"));
        return;
    }

    bool isIPv4 = wxRegEx("^((?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?))(?::\\d+)?$").Matches(m_tcpStunCtrl->GetValue());
    bool isIPv6 = wxRegEx("^(\\[([a-fA-F0-9:]+)\\](?::\\d+)?|(?<!\\[)([a-fA-F0-9:]+)(?!\\]))$").Matches(m_tcpStunCtrl->GetValue());

    if (!isIPv4 && !isIPv6)
    {
        wxMenu *menu = new wxMenu();
        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent &)
                   { doExploreNat(WebPier::Context::Service::TCP, wxT("0.0.0.0:0")); }, menu->Append(wxID_ANY, _("IPv&4"))->GetId());

        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent &)
                   { doExploreNat(WebPier::Context::Service::TCP, wxT("[::]:0")); }, menu->Append(wxID_ANY, _("IPv&6"))->GetId());

        PopupMenu(menu);
    }
    else
    {
        doExploreNat(WebPier::Context::Service::TCP, isIPv4 ? wxT("0.0.0.0:0") : wxT("[::]:0"));
    }
}

void CSettingsDialog::onDhtTestClick(wxCommandEvent &event)
{
    std::weak_ptr<CSettingsDialog> weak = shared_from_this();

    wxUint32 port = 0;
    m_dhtPortCtrl->GetValue().ToUInt(&port);
    WebPier::Utils::CheckDhtRendezvous(m_dhtBootCtrl->GetValue(), 0, port, [this, weak](const wxString &error)
    {
        if(auto ptr = weak.lock())
        {
            ptr->CallAfter([this, ptr, error]()
            {
                m_dhtTest->Enable();
                m_dhtGauge->SetValue(0);
                m_dhtGauge->Hide();
                if (error.IsEmpty())
                {
                    m_dhtTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON)));
                    m_dhtPanel->SetToolTip(_("DHT rendezvous is Ok!"));
                }
                else
                {
                    m_dhtTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON)));
                    m_dhtPanel->SetToolTip(wxString::Format(_("DHT rendezvous is wrong!\n%s"), error));
                }
            });
        } 
    });
    m_dhtGauge->Show();
    m_dhtGauge->Pulse();
    m_dhtTest->Disable();
}

void CSettingsDialog::onEmailTestClick(wxCommandEvent &event)
{
    auto pier = m_ownerCtrl->GetValue() + '/' + m_pierCtrl->GetValue();
    if (m_config->Pier != pier)
    {
        CMessageDialog dialog(this, _("The identity has been changed, but the context hasn't been replaced yet. You should apply changes before this test."), wxDEFAULT_DIALOG_STYLE | wxICON_WARNING);
        dialog.ShowModal();
        return;
    }

    std::weak_ptr<CSettingsDialog> weak = shared_from_this();
    WebPier::Utils::CheckEmailRendezvous(m_smtpCtrl->GetValue(), m_imapCtrl->GetValue(), m_loginCtrl->GetValue(), m_passCtrl->GetValue(), m_certPicker->GetPath(), m_keyPicker->GetPath(), m_caPicker->GetPath(), [this, weak](const wxString &error)
    {
        if(auto ptr = weak.lock())
        {
            ptr->CallAfter([this, ptr, error]()
            {
                m_emailTest->Enable();
                m_emailGauge->SetValue(0);
                m_emailGauge->Hide();
                if (error.IsEmpty())
                {
                    m_emailTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON)));
                    m_emailPanel->SetToolTip(_("Email rendezvous is Ok!"));
                }
                else
                {
                    m_emailTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON)));
                    m_emailPanel->SetToolTip(wxString::Format(_("Email rendezvous is wrong!\n%s"), error));
                }
            });
        } 
    });
    m_emailGauge->Show();
    m_emailGauge->Pulse();
    m_emailTest->Disable();
}

void CSettingsDialog::onOkButtonClick(wxCommandEvent &event)
{
    if (m_ownerCtrl->GetValue().IsEmpty() || m_pierCtrl->GetValue().IsEmpty() || (m_udpStunCtrl->GetValue().IsEmpty() && m_tcpStunCtrl->GetValue().IsEmpty()) || (m_dhtBootCtrl->GetValue().IsEmpty() && (m_smtpCtrl->GetValue().IsEmpty() || m_imapCtrl->GetValue().IsEmpty() || m_loginCtrl->GetValue().IsEmpty() || m_passCtrl->GetValue().IsEmpty())))
    {
        CMessageDialog dialog(this, _("To use the WebPier you must define the identity, STUN and either DHT or Email rendezvous"), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
        return;
    }

    auto pier = m_ownerCtrl->GetValue() + '/' + m_pierCtrl->GetValue();
    if (!m_config->Pier.IsEmpty() && pier != m_config->Pier)
    {
        CMessageDialog dialog(this, _("You have changed your WebPier identity. This will cause the context to switch. The current services will become unavailable. Continue?"), wxDEFAULT_DIALOG_STYLE | wxICON_WARNING | wxICON_QUESTION);
        if (dialog.ShowModal() == wxID_NO)
            return;
    }

    if (m_daemon != m_daemonCtrl->GetValue())
    {
        try
        {
            m_daemon
                ? WebPier::Backend::RevokeAutostart()
                : WebPier::Backend::AssignAutostart();
        }
        catch (const std::exception &ex)
        {
            CMessageDialog dialog(nullptr, _("Can't change the backend startup mode. ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
            dialog.ShowModal();
        }
    }

    m_config->Pier = pier;

    m_config->UdpStunServer = m_udpStunCtrl->GetValue();
    m_config->TcpStunServer = m_tcpStunCtrl->GetValue();
    m_config->NatTest = m_isNat->GetValue() 
        ? static_cast<WebPier::Context::Config::Checkup>(m_testMode->GetSelection() + 1) 
        : WebPier::Context::Config::Checkup::Noneed;
    uint32_t hops = m_config->NatHops;
    m_hopsCtrl->GetValue().ToUInt(&hops);
    m_config->NatHops = static_cast<uint8_t>(hops);

    m_config->DhtBootstrap = m_dhtBootCtrl->GetValue();
    uint32_t port = m_config->DhtPort;
    m_dhtPortCtrl->GetValue().ToUInt(&port);
    m_config->DhtPort = port;

    m_config->SmtpServer = m_smtpCtrl->GetValue();
    m_config->ImapServer = m_imapCtrl->GetValue();
    m_config->EmailLogin = m_loginCtrl->GetValue();
    m_config->EmailPassword = m_passCtrl->GetValue();
    m_config->EmailX509Cert = m_certPicker->GetPath();
    m_config->EmailX509Key = m_keyPicker->GetPath();
    m_config->EmailX509Ca = m_caPicker->GetPath();

    event.Skip();
}
