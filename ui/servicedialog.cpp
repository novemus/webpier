#include "servicedialog.h"

CServiceDialog::CServiceDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style)
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    wxString rendChoices[] = { _("DHT"), _("Email") };

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    m_propGrid = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_BOLD_MODIFIED|wxPG_HIDE_MARGIN);
    m_idItem = m_propGrid->Append( new wxStringProperty( _("Service") ) );
    m_peerItem = m_propGrid->Append( new wxMultiChoiceProperty( _("Peer"), wxPG_LABEL));
    m_mapItem = m_propGrid->Append( new wxStringProperty( _("Mapping") ) );
    m_gateItem = m_propGrid->Append( new wxStringProperty( _("Gateway"), wxPG_LABEL, _("0.0.0.0:0") ) );
    m_startItem = m_propGrid->Append( new wxBoolProperty( _("Autostart") ) );
    m_obsItem = m_propGrid->Append( new wxBoolProperty( _("Obscure"), wxPG_LABEL, true ) );
    m_rendItem = m_propGrid->Append( new wxEnumProperty( _("Rendezvous"), wxPG_LABEL, wxArrayString(2, rendChoices) ) );
    m_bootItem = m_rendItem->InsertChild( 0, new wxStringProperty( _("Bootstrap") ) );
    m_netItem = m_rendItem->InsertChild( 1, new wxStringProperty( _("Network") ) );
    mainSizer->Add( m_propGrid, 1, wxALL|wxEXPAND, 5 );

    wxStdDialogButtonSizer* sdbSizer;
    sdbSizer = new wxStdDialogButtonSizer();
    m_ok = new wxButton( this, wxID_OK );
    sdbSizer->AddButton( m_ok );
    m_cancel = new wxButton( this, wxID_CANCEL );
    sdbSizer->AddButton( m_cancel );
    sdbSizer->Realize();

    mainSizer->Add( sdbSizer, 0, wxEXPAND|wxTOP|wxBOTTOM, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    m_propGrid->Connect( wxEVT_PG_CHANGED, wxPropertyGridEventHandler( CServiceDialog::onPropertyChanged ), NULL, this );
}

CServiceDialog::~CServiceDialog()
{
    delete m_propGrid;
    delete m_ok;
    delete m_cancel;
}

void CServiceDialog::onPropertyChanged( wxPropertyGridEvent& event )
{
    if (event.GetProperty() == m_rendItem)
    {
        int choice = m_rendItem->GetChoiceSelection();
        if (choice == 0)
        {
            m_bootItem = m_rendItem->InsertChild( 0, new wxStringProperty( _("Bootstrap") ) );
            m_netItem = m_rendItem->InsertChild( 1, new wxStringProperty( _("Network") ) );
        }
        else
        {
            m_rendItem->DeleteChildren();
            m_bootItem = nullptr;
            m_netItem = nullptr;
        }
        this->Layout();
    }
}
