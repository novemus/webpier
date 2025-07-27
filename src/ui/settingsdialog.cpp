#include <ui/settingsdialog.h>
#include <ui/messagedialog.h>
#include <ui/logo.h>
#include <wx/msgdlg.h> 
#include <wx/valnum.h>
#include <wx/datetime.h>
#include <wx/notifmsg.h>
#include <cstdint>

#ifdef WIN32
#include <Shlobj_core.h>
#endif

wxString ToString(const WebPier::Utils::NatState::Binding& value)
{
    switch (value)
    {
        case WebPier::Utils::NatState::PortDependent:
            return _("port dependent");
        case WebPier::Utils::NatState::AddressDependent:
            return _("address dependent");
        case WebPier::Utils::NatState::AddressAndPortDependent:
            return _("address and port dependent");
        default:
            return _("independent");
    }
    return wxEmptyString;
}

wxString ToString(bool value)
{
    return value ? _("yes") : _("no");
}

CSettingsDialog::CSettingsDialog(WebPier::Context::ConfigPtr config, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(nullptr, wxID_ANY, title, pos, size, style)
    , m_config(config)
    , m_daemon(WebPier::Backend::VerifyAutostart())
{
    static constexpr const char* FORBIDDEN_PATH_CHARS = "*/\\<>:|? ";

    this->SetIcon(::GetAppIconBundle().GetIcon());
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

    m_ownerCtrl = new wxTextCtrl( idSizer->GetStaticBox(), wxID_ANY, m_config->Pier.Before('/'), wxDefaultPosition, wxDefaultSize, 0);
    m_ownerCtrl->SetToolTip( _("The email address that represents you to owners of remote piers") );
    m_ownerCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator*)m_ownerCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add( m_ownerCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* pierLabel;
    pierLabel = new wxStaticText( idSizer->GetStaticBox(), wxID_ANY, _("Pier"), wxDefaultPosition, wxDefaultSize, 0 );
    pierLabel->Wrap( -1 );
    idGridSizer->Add( pierLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_pierCtrl = new wxTextCtrl( idSizer->GetStaticBox(), wxID_ANY, m_config->Pier.After('/'), wxDefaultPosition, wxDefaultSize, 0 );
    m_pierCtrl->SetToolTip( _("The identifier of this pier") );
    m_pierCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator*)m_pierCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add( m_pierCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
    idSizer->Add( idGridSizer, 1, wxALL|wxEXPAND, 5 );
    basicSizer->Add( idSizer, 0, wxEXPAND|wxALL, 10 );

    m_daemonCtrl = new wxCheckBox( basicPanel, wxID_ANY, _("Run the backend at system startup"), wxDefaultPosition, wxDefaultSize, 0 );
    m_daemonCtrl->SetValue(m_daemon);
    m_daemonCtrl->SetToolTip( _("The change will take effect after the system is restarted") );
    basicSizer->Add( m_daemonCtrl, 0, wxALL, 5 );

    basicPanel->SetSizer( basicSizer );
    basicPanel->Layout();
    basicSizer->Fit( basicPanel );
    m_notebook->AddPage( basicPanel, _("Basic"), true );
    m_natPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_natPanel->SetToolTip( _("NAT traverse settings") );

    wxBoxSizer* stunSizer;
    stunSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer *stunGridSizer;
    stunGridSizer = new wxFlexGridSizer(0, 2, 5, 5);
    stunGridSizer->AddGrowableCol(1);
    stunGridSizer->SetFlexibleDirection(wxHORIZONTAL);
    stunGridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxStaticText *stunLabel;
    stunLabel = new wxStaticText(m_natPanel, wxID_ANY, _("STUN server"), wxDefaultPosition, wxSize(100, -1), 0);
    stunLabel->Wrap(-1);
    stunGridSizer->Add(stunLabel, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxRIGHT | wxLEFT, 5);

    m_stunCtrl = new wxTextCtrl(m_natPanel, wxID_ANY, _("stun.ekiga.net"), wxDefaultPosition, wxDefaultSize, 0);
    stunGridSizer->Add(m_stunCtrl, 1, wxTOP | wxRIGHT | wxLEFT | wxEXPAND | wxALIGN_CENTER_VERTICAL, 5);

    wxStaticText *punchLabel;
    punchLabel = new wxStaticText(m_natPanel, wxID_ANY, _("Punch hops"), wxDefaultPosition, wxSize(100, -1), 0);
    punchLabel->Wrap(-1);
    stunGridSizer->Add(punchLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_punchCtrl = new wxTextCtrl(m_natPanel, wxID_ANY, _("7"), wxDefaultPosition, wxDefaultSize, 0);
    stunGridSizer->Add(m_punchCtrl, 0, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxRIGHT | wxLEFT, 5);

    m_stunTest = new wxButton(m_natPanel, wxID_ANY, _("Test"), wxDefaultPosition, wxDefaultSize, 0);
    m_stunTest->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    stunGridSizer->Add(m_stunTest, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL, 10);

    m_stunGauge = new wxGauge(m_natPanel, wxID_ANY, 20, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL);
    m_stunGauge->SetValue(0);
    m_stunGauge->Enable(false);

    stunGridSizer->Add(m_stunGauge, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    stunSizer->Add(stunGridSizer, 0, wxEXPAND | wxALL, 5);

    m_natPanel->SetSizer( stunSizer );
    m_natPanel->Layout();
    stunSizer->Fit( m_natPanel );
    m_notebook->AddPage( m_natPanel, _("NAT"), false );
    wxPanel* dhtPanel;
    dhtPanel = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    dhtPanel->SetToolTip( _("DHT node settings") );

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

    m_dhtBootCtrl = new wxTextCtrl( dhtPanel, wxID_ANY, m_config->DhtBootstrap, wxDefaultPosition, wxSize( -1,-1 ), 0 );
    dhtGridSizer->Add( m_dhtBootCtrl, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

    wxStaticText* dhtPortLabel;
    dhtPortLabel = new wxStaticText( dhtPanel, wxID_ANY, _("Port"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    dhtPortLabel->Wrap( -1 );
    dhtGridSizer->Add( dhtPortLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_dhtPortCtrl = new wxTextCtrl( dhtPanel, wxID_ANY, wxString::Format(wxT("%d"), (int)m_config->DhtPort), wxDefaultPosition, wxDefaultSize, 0 );
    m_dhtPortCtrl->SetValidator( wxIntegerValidator<unsigned int>() );
    dhtGridSizer->Add( m_dhtPortCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

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

    m_smtpCtrl = new wxTextCtrl( emailPanel, wxID_ANY, m_config->SmtpServer, wxDefaultPosition, wxDefaultSize, 0 );
    emailGridSizer->Add( m_smtpCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    wxStaticText* imapLabel;
    imapLabel = new wxStaticText( emailPanel, wxID_ANY, _("IMAP"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    imapLabel->Wrap( -1 );
    emailGridSizer->Add( imapLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_imapCtrl = new wxTextCtrl( emailPanel, wxID_ANY, m_config->ImapServer, wxDefaultPosition, wxDefaultSize, 0 );
    emailGridSizer->Add( m_imapCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* loginLabel;
    loginLabel = new wxStaticText( emailPanel, wxID_ANY, _("Login"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    loginLabel->Wrap( -1 );
    emailGridSizer->Add( loginLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_loginCtrl = new wxTextCtrl( emailPanel, wxID_ANY, m_config->EmailLogin, wxDefaultPosition, wxDefaultSize, 0 );
    emailGridSizer->Add( m_loginCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* passLabel;
    passLabel = new wxStaticText( emailPanel, wxID_ANY, _("Password"), wxDefaultPosition, wxDefaultSize, 0 );
    passLabel->Wrap( -1 );
    passLabel->SetMinSize( wxSize( 100,-1 ) );

    emailGridSizer->Add( passLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_passCtrl = new wxTextCtrl( emailPanel, wxID_ANY, m_config->EmailPassword, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
    emailGridSizer->Add( m_passCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* certLabel;
    certLabel = new wxStaticText( emailPanel, wxID_ANY, _("Certificate"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    certLabel->Wrap( -1 );
    emailGridSizer->Add( certLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_certPicker = new wxFilePickerCtrl( emailPanel, wxID_ANY, m_config->EmailX509Cert, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
    emailGridSizer->Add( m_certPicker, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* keyLabel;
    keyLabel = new wxStaticText( emailPanel, wxID_ANY, _("Key"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    keyLabel->Wrap( -1 );
    emailGridSizer->Add( keyLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_keyPicker = new wxFilePickerCtrl( emailPanel, wxID_ANY, m_config->EmailX509Key, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
    emailGridSizer->Add( m_keyPicker, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    wxStaticText* caLabel;
    caLabel = new wxStaticText( emailPanel, wxID_ANY, _("CA"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    caLabel->Wrap( -1 );
    emailGridSizer->Add( caLabel, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_caPicker = new wxFilePickerCtrl( emailPanel, wxID_ANY, m_config->EmailX509Ca, _("Select a file"), _("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
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

    footSizer->Add( m_cancelBtn, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
    m_okBtn = new wxButton( this, wxID_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );

#ifndef WIN32
    m_cancelBtn->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON) ) );
    m_okBtn->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON) ) );
#endif

    footSizer->Add( m_okBtn, 0, wxALL, 5 );

    mainSizer->Add( footSizer, 0, wxEXPAND, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    this->Connect( wxEVT_IDLE, wxIdleEventHandler( CSettingsDialog::onIdle ) );
    m_okBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CSettingsDialog::onOkButtonClick ), NULL, this );

#ifdef WIN32
    m_daemonCtrl->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CSettingsDialog::onDaemonCheckBoxClick ), NULL, this );
#endif

    m_stunCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( CSettingsDialog::onStunChange ), NULL, this );
    m_stunTest->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CSettingsDialog::onStunTestClick ), NULL, this );
}

CSettingsDialog::~CSettingsDialog()
{
    delete m_notebook;
    delete m_cancelBtn;
    delete m_okBtn;
}

void CSettingsDialog::onIdle(wxIdleEvent& event)
{
    if (m_stunGauge->IsEnabled())
        m_stunGauge->SetValue(wxDateTime::Now().GetSecond() % 60 * 3 % 20);

    event.Skip();
}

void CSettingsDialog::onStunChange(wxCommandEvent& event)
{
    m_stunTest->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_REDO), wxASCII_STR(wxART_BUTTON)));
    m_natPanel->SetToolTip(wxEmptyString);
    event.Skip();
}

void CSettingsDialog::onStunTestClick(wxCommandEvent& event)
{
    std::weak_ptr<CSettingsDialog> weak = shared_from_this();
    WebPier::Utils::ExploreNat(wxT("0.0.0.0"), m_stunCtrl->GetValue(), [this, weak](const WebPier::Utils::NatState& state)
    {
        if(auto ptr = weak.lock())
        {
            ptr->CallAfter([this, ptr, state]()
            {
                m_stunTest->Enable();
                m_stunGauge->SetValue(0);
                m_stunGauge->Disable();
                if (state.Error.IsEmpty())
                {
                    m_stunTest->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(state.Mapping == WebPier::Utils::NatState::Independent ? wxART_TICK_MARK : wxART_WARNING), wxASCII_STR(wxART_BUTTON)));
                    m_natPanel->SetToolTip(wxString::Format(_("%s\n\nNAT: %s\nHairpin: %s\nRandom port: %s\nVariable IP: %s\nMapping: %s\nFiltering: %s\nInner EP: %s\nMapped EP: %s"),
                            m_stunCtrl->GetValue(),
                            ToString(state.Nat),
                            ToString(state.Hairpin),
                            ToString(state.RandomPort),
                            ToString(state.VariableAddress),
                            ToString(state.Mapping),
                            ToString(state.Filtering),
                            state.InnerEndpoint,
                            state.OuterEndpoint
                        ));
                }
                else
                {
                    m_stunTest->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON)));
                    m_natPanel->SetToolTip(wxString::Format(wxT("%s\n\n%s"), m_stunCtrl->GetValue(), state.Error));
                }
            });
        }
    });
    m_stunGauge->Enable();
    m_stunGauge->SetValue(0);
    m_stunTest->Disable();
}

void CSettingsDialog::onDaemonCheckBoxClick(wxCommandEvent& event)
{
#ifdef WIN32
    if (!IsUserAnAdmin())
    {
        CMessageDialog dialog(this, _("Run the WebPier with administrator privileges to change this option."), wxDEFAULT_DIALOG_STYLE|wxICON_WARNING);
        dialog.ShowModal();
        m_daemonCtrl->SetValue(m_daemon);
        return;
    }
#endif
    event.Skip();
}

void CSettingsDialog::onOkButtonClick(wxCommandEvent& event)
{
    if (m_ownerCtrl->GetValue().IsEmpty() || m_pierCtrl->GetValue().IsEmpty() || m_stunCtrl->GetValue().IsEmpty() 
        || (m_dhtBootCtrl->GetValue().IsEmpty() && (m_smtpCtrl->GetValue().IsEmpty() || m_imapCtrl->GetValue().IsEmpty() 
            || m_loginCtrl->GetValue().IsEmpty() || m_passCtrl->GetValue().IsEmpty())))
    {
        CMessageDialog dialog(this, _("To use the WebPier you must define the identity, STUN and either DHT or Email rendezvous"), wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
        dialog.ShowModal();
        return;
    }

    auto pier = m_ownerCtrl->GetValue() + '/' + m_pierCtrl->GetValue();
    if (!m_config->Pier.IsEmpty() && pier != m_config->Pier)
    {
        CMessageDialog dialog(this, _("You have changed your WebPier identity. This will cause the context to switch. The current services will become unavailable. To switch the context back you must restore the old identity."), wxDEFAULT_DIALOG_STYLE|wxICON_WARNING);
        dialog.ShowModal();
    }

    if (m_daemon != m_daemonCtrl->GetValue())
    {
        try
        {
            m_daemon 
                ? WebPier::Backend::RevokeAutostart() 
                : WebPier::Backend::AssignAutostart();
        }
        catch (const std::exception& ex) 
        {
            CMessageDialog dialog(nullptr, _("Can't change the backend startup mode. ") + ex.what(), wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
            dialog.ShowModal();
        }
    }

    m_config->Pier = pier;

    m_config->StunServer = m_stunCtrl->GetValue();
    uint32_t hops = m_config->PunchHops;
    m_punchCtrl->GetValue().ToUInt(&hops);
    m_config->PunchHops = static_cast<uint8_t>(hops);

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

    m_config->Store();
    event.Skip();
}
