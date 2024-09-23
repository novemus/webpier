#include "aboutdialog.h"
#include "logo.h"

CAboutDialog::CAboutDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    m_panel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN|wxTAB_TRAVERSAL );
    m_panel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );

    wxBoxSizer* panelSizer;
    panelSizer = new wxBoxSizer( wxVERTICAL );

    m_logo = new wxStaticBitmap( m_panel, wxID_ANY, ::GetLogo(), wxDefaultPosition, wxDefaultSize, 0 );
    panelSizer->Add( m_logo, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

    m_appName = new wxStaticText( m_panel, wxID_ANY, _("WebPier"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    m_appName->Wrap( -1 );
    m_appName->SetFont( wxFont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

    panelSizer->Add( m_appName, 1, wxRIGHT|wxLEFT|wxALIGN_CENTER_HORIZONTAL, 5 );

    m_appVersion = new wxStaticText( m_panel, wxID_ANY, _("1.0.0-beta"), wxDefaultPosition, wxDefaultSize, 0 );
    m_appVersion->Wrap( -1 );
    panelSizer->Add( m_appVersion, 0, wxRIGHT|wxLEFT|wxALIGN_CENTER_HORIZONTAL, 5 );

    m_appPurpose = new wxStaticText( m_panel, wxID_ANY, _("A program for mapping remote TCP services placed behind NAT to a local interface"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL );
    m_appPurpose->Wrap( -1 );
    panelSizer->Add( m_appPurpose, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 20 );

	m_homeLabel = new wxStaticText( m_panel, wxID_ANY, _("Homepage:"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_homeLabel->Wrap( -1 );
	m_homeLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	panelSizer->Add( m_homeLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxLEFT, 5 );

	m_homepage = new wxHyperlinkCtrl( m_panel, wxID_ANY, _("https://github.com/novemus/webpier"), wxT("https://github.com/novemus/webpier"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	panelSizer->Add( m_homepage, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxLEFT, 5 );

	panelSizer->Add( 0, 0, 1, 0, 5 );

	m_copyLabel = new wxStaticText( m_panel, wxID_ANY, _("Copyright:"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_copyLabel->Wrap( -1 );
	m_copyLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

	panelSizer->Add( m_copyLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxLEFT, 5 );

	m_copyright = new wxStaticText( m_panel, wxID_ANY, _("2024 Novemus Band"), wxDefaultPosition, wxDefaultSize, 0 );
	m_copyright->Wrap( -1 );
	panelSizer->Add( m_copyright, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_panel->SetSizer( panelSizer );
	m_panel->Layout();
	panelSizer->Fit( m_panel );
	mainSizer->Add( m_panel, 1, wxEXPAND | wxALL, 5 );

    wxStdDialogButtonSizer* sdbSizer;
    sdbSizer = new wxStdDialogButtonSizer();
    m_oK = new wxButton( this, wxID_OK );
    sdbSizer->AddButton( m_oK );
    sdbSizer->Realize();

    mainSizer->Add( sdbSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxBOTTOM, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );
}

CAboutDialog::~CAboutDialog()
{
    delete m_panel;
    delete m_oK;
}
