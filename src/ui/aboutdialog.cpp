#include <ui/aboutdialog.h>
#include <ui/logo.h>

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

    wxBoxSizer* topSizer;
    topSizer = new wxBoxSizer( wxHORIZONTAL );

    wxBoxSizer* titleSizer;
    titleSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticText* appName = new wxStaticText( m_panel, wxID_ANY, wxT("WebPier"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    appName->Wrap( -1 );
    appName->SetFont( wxFont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );

    titleSizer->Add( appName, 0, 0, 5 );

    wxStaticText* appVersion = new wxStaticText( m_panel, wxID_ANY, wxT("1.0.0-beta"), wxDefaultPosition, wxDefaultSize, 0 );
    appVersion->Wrap( -1 );
    titleSizer->Add( appVersion, 0, 0, 5 );

    topSizer->Add( titleSizer, 1, wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 10 );

    wxStaticBitmap* logo = new wxStaticBitmap( m_panel, wxID_ANY, ::GetLogo(), wxDefaultPosition, wxDefaultSize, 0 );
    topSizer->Add( logo, 0, wxTOP|wxRIGHT|wxLEFT, 10 );

    panelSizer->Add( topSizer, 1, wxEXPAND|wxALL, 10 );

    wxStaticText* appPurpose = new wxStaticText( m_panel, wxID_ANY, _("A program for forwarding TCP services running on machines located behind the NAT"), wxDefaultPosition, wxDefaultSize, 0 );
    appPurpose->Wrap( -1 );
    panelSizer->Add( appPurpose, 0, wxALL, 20 );

    wxFlexGridSizer* bottomSizer;
    bottomSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
    bottomSizer->SetFlexibleDirection( wxBOTH );
    bottomSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText* homeLabel = new wxStaticText( m_panel, wxID_ANY, _("Homepage:"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    homeLabel->Wrap( -1 );
    homeLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

    bottomSizer->Add( homeLabel, 0, wxRIGHT|wxLEFT, 10 );

    wxHyperlinkCtrl* homepage = new wxHyperlinkCtrl( m_panel, wxID_ANY, wxT("https://github.com/novemus/webpier"), wxT("https://github.com/novemus/webpier"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    bottomSizer->Add( homepage, 0, wxRIGHT, 10 );

    wxStaticText* copyLabel = new wxStaticText( m_panel, wxID_ANY, _("Copyright:"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
    copyLabel->Wrap( -1 );
    copyLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );

    bottomSizer->Add( copyLabel, 0, wxRIGHT|wxLEFT, 10 );

    wxStaticText* copyright = new wxStaticText( m_panel, wxID_ANY, _("2024 Novemus Band"), wxDefaultPosition, wxDefaultSize, 0 );
    copyright->Wrap( -1 );
    bottomSizer->Add( copyright, 0, wxBOTTOM|wxRIGHT, 10 );

    panelSizer->Add( bottomSizer, 0, wxEXPAND|wxALL, 10 );

    m_panel->SetSizer( panelSizer );
    m_panel->Layout();
    panelSizer->Fit( m_panel );
    mainSizer->Add( m_panel, 1, wxEXPAND | wxALL, 5 );

    m_ok = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
#ifndef WIN32
    m_ok->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON) ) );
#endif
    mainSizer->Add( m_ok, 0, wxALIGN_RIGHT|wxALL, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );
}

CAboutDialog::~CAboutDialog()
{
    delete m_panel;
    delete m_ok;
}
