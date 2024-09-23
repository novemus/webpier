#include "exportdialog.h"

///////////////////////////////////////////////////////////////////////////

CExportDialog::CExportDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    wxStaticBoxSizer* peerSizer;
    peerSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("indefinite") ), wxVERTICAL );

    wxBoxSizer* listSizer;
    listSizer = new wxBoxSizer( wxHORIZONTAL );

    m_serviceList = new wxCheckListBox( peerSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,10 ), 0, nullptr, wxLB_SINGLE );
    listSizer->Add( m_serviceList, 0, wxALL|wxEXPAND, 5 );

    wxBoxSizer* tableSizer;
    tableSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* serviceSizer;
    serviceSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
    serviceSizer->AddGrowableCol( 1 );
    serviceSizer->SetFlexibleDirection( wxHORIZONTAL );
    serviceSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_idLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Id"), wxDefaultPosition, wxDefaultSize, 0 );
    m_idLabel->Wrap( -1 );
    serviceSizer->Add( m_idLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_idValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("ssh-service"), wxDefaultPosition, wxDefaultSize, 0 );
    m_idValue->Wrap( -1 );
    serviceSizer->Add( m_idValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_serviceLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Service"), wxDefaultPosition, wxDefaultSize, 0 );
    m_serviceLabel->Wrap( -1 );
    m_serviceLabel->SetToolTip( _("Local endpoint to map remote service") );

    serviceSizer->Add( m_serviceLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_serviceValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("127.0.0.1:22"), wxDefaultPosition, wxDefaultSize, 0 );
    m_serviceValue->Wrap( -1 );
    serviceSizer->Add( m_serviceValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_gateLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Gateway"), wxDefaultPosition, wxDefaultSize, 0 );
    m_gateLabel->Wrap( -1 );
    m_gateLabel->SetToolTip( _("Local endpoint to bind wormhole tunnel") );

    serviceSizer->Add( m_gateLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_gateValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("0.0.0.0:0"), wxDefaultPosition, wxDefaultSize, 0 );
    m_gateValue->Wrap( -1 );
    serviceSizer->Add( m_gateValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_startLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Autostart"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startLabel->Wrap( -1 );
    serviceSizer->Add( m_startLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_startValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("no"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startValue->Wrap( -1 );
    serviceSizer->Add( m_startValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_obscureLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Obscure"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureLabel->Wrap( -1 );
    serviceSizer->Add( m_obscureLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_obscureValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("yes"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureValue->Wrap( -1 );
    serviceSizer->Add( m_obscureValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    tableSizer->Add( serviceSizer, 1, wxEXPAND|wxALL, 5 );

    wxStaticBoxSizer* rendSizer;
    rendSizer = new wxStaticBoxSizer( new wxStaticBox( peerSizer->GetStaticBox(), wxID_ANY, _("DHT") ), wxVERTICAL );

    wxFlexGridSizer* rendGridSizer;
    rendGridSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
    rendGridSizer->SetFlexibleDirection( wxHORIZONTAL );
    rendGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_bootLabel = new wxStaticText( rendSizer->GetStaticBox(), wxID_ANY, _("Bootstrap"), wxDefaultPosition, wxDefaultSize, 0 );
    m_bootLabel->Wrap( -1 );
    rendGridSizer->Add( m_bootLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_bootValue = new wxStaticText( rendSizer->GetStaticBox(), wxID_ANY, _("bootstrap.jami.net:4222"), wxDefaultPosition, wxDefaultSize, 0 );
    m_bootValue->Wrap( -1 );
    rendGridSizer->Add( m_bootValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_netLabel = new wxStaticText( rendSizer->GetStaticBox(), wxID_ANY, _("Network"), wxDefaultPosition, wxDefaultSize, 0 );
    m_netLabel->Wrap( -1 );
    rendGridSizer->Add( m_netLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_netValue = new wxStaticText( rendSizer->GetStaticBox(), wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, 0 );
    m_netValue->Wrap( -1 );
    rendGridSizer->Add( m_netValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    rendSizer->Add( rendGridSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    tableSizer->Add( rendSizer, 0, wxEXPAND|wxALL, 5 );
    listSizer->Add( tableSizer, 1, wxEXPAND, 5 );
    peerSizer->Add( listSizer, 1, wxALL|wxEXPAND, 5 );
    mainSizer->Add( peerSizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 10 );

    wxStdDialogButtonSizer* sdbSizer;
    sdbSizer = new wxStdDialogButtonSizer();
    m_ok = new wxButton( this, wxID_OK );
    sdbSizer->AddButton( m_ok );
    m_cancel = new wxButton( this, wxID_CANCEL );
    sdbSizer->AddButton( m_cancel );
    sdbSizer->Realize();

    mainSizer->Add( sdbSizer, 0, wxEXPAND|wxBOTTOM, 10 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    // Connect Events
    m_serviceList->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( CExportDialog::onListItemSelected ), NULL, this );
}

CExportDialog::~CExportDialog()
{
    delete m_serviceList;
    delete m_idLabel;
    delete m_idValue;
    delete m_serviceLabel;
    delete m_serviceValue;
    delete m_gateLabel;
    delete m_gateValue;
    delete m_startLabel;
    delete m_startValue;
    delete m_obscureLabel;
    delete m_obscureValue;
    delete m_bootLabel;
    delete m_bootValue;
    delete m_netLabel;
    delete m_netValue;
    delete m_ok;
    delete m_cancel;
}
