#include "servicedialog.h"
#include "messagedialog.h"
#include <wx/valnum.h>

CServiceDialog::CServiceDialog(WebPier::ConfigPtr config, WebPier::ServicePtr service, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style)
    , m_config(config)
    , m_service(service)
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    wxArrayString rendChoice;
    rendChoice.Add(wxT("DHT"));
    rendChoice.Add(wxT("Email"));

    auto peerChoice = WebPier::GetPeers();

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    m_propGrid = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_BOLD_MODIFIED|wxPG_HIDE_MARGIN);
    m_nameItem = m_propGrid->Append( new wxStringProperty( _("Name"), wxPG_LABEL, m_service->Name ) );
    if (m_service->IsExport())
    {
        m_peerItem = m_propGrid->Append(new wxMultiChoiceProperty( _("Peer"), wxPG_LABEL, peerChoice, wxSplit(m_service->Peer, ' ')));
    }
    else
    {
        int value = 0;
        for(auto& peer : peerChoice)
        {
            if (peer == m_service->Peer)
                break;
            ++value;
        }
        m_peerItem = m_propGrid->Append( new wxEnumProperty( _("Peer"), wxPG_LABEL, peerChoice, wxArrayInt(), value ) );
    }

    m_addrItem = m_propGrid->Append( new wxStringProperty( _("Address"), wxPG_LABEL, m_service->Address ) );
    m_gateItem = m_propGrid->Append( new wxStringProperty( _("Gateway"), wxPG_LABEL, m_service->Gateway ) );
    m_startItem = m_propGrid->Append( new wxBoolProperty( _("Autostart"), wxPG_LABEL, m_service->Autostart ) );
    m_obsItem = m_propGrid->Append( new wxBoolProperty( _("Obscure"), wxPG_LABEL, m_service->Obscure ) );
    m_rendItem = m_propGrid->Append( new wxEnumProperty( _("Rendezvous"), wxPG_LABEL, rendChoice, wxArrayInt(), m_service->DhtBootstrap.IsEmpty() ? 1 : 0 ) );

    if (!m_service->DhtBootstrap.IsEmpty())
    {
        m_bootItem = m_rendItem->InsertChild( 0, new wxStringProperty( _("Bootstrap"), wxPG_LABEL, m_service->DhtBootstrap ) );
        m_netItem = m_rendItem->InsertChild( 1, new wxUIntProperty( _("Network"), wxPG_LABEL, m_service->DhtNetwork ) );
    }

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
    m_ok->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CServiceDialog::onOKButtonClick ), NULL, this );
}

CServiceDialog::~CServiceDialog()
{
    delete m_propGrid;
    delete m_ok;
    delete m_cancel;
}

void CServiceDialog::onPropertyChanged( wxPropertyGridEvent& event )
{
    auto* prop = event.GetProperty();
    if (prop == m_rendItem)
    {
        int choice = m_rendItem->GetChoiceSelection();
        if (choice == 0)
        {
            auto bootstrap = m_service->DhtBootstrap.IsEmpty() ? m_config->DhtBootstrap : m_service->DhtBootstrap;
            auto network = m_service->DhtBootstrap.IsEmpty() ? m_config->DhtNetwork : m_service->DhtNetwork;

            m_bootItem = m_rendItem->InsertChild( 0, new wxStringProperty( _("Bootstrap"), wxPG_LABEL, bootstrap ) );
            m_netItem = m_rendItem->InsertChild( 1, new wxUIntProperty( _("Network"), wxPG_LABEL, network ) );
        }
        else
        {
            m_rendItem->DeleteChildren();
            m_bootItem = nullptr;
            m_netItem = nullptr;
        }
    }

    this->Layout();
}

void CServiceDialog::onOKButtonClick( wxCommandEvent& event )
{
    m_service->Name = m_nameItem->GetValueAsString();
    if (m_service->IsExport())
        m_service->Peer = wxJoin(m_peerItem->GetValue().GetArrayString(), ' ');
    else
        m_service->Peer = m_peerItem->GetValueAsString();
    m_service->Address = m_addrItem->GetValueAsString();
    m_service->Gateway = m_gateItem->GetValueAsString();
    m_service->Obscure = m_obsItem->GetValue().GetBool();
    m_service->Autostart = m_startItem->GetValue().GetBool();
    m_service->DhtBootstrap = m_bootItem ? m_bootItem->GetValueAsString() : "";
    m_service->DhtNetwork = m_netItem ? m_netItem->GetValue().GetULongLong().GetLo() : 0;

    if (m_service->Name.IsEmpty() || m_service->Address.IsEmpty() || m_service->Gateway.IsEmpty() || (m_service->IsImport() && m_service->Peer.IsEmpty()) || (m_bootItem && m_service->DhtBootstrap.IsEmpty()))
    {
        wxString message = m_service->IsImport() 
            ? (m_bootItem ? _("Define the 'name', 'peer', 'address', 'gateway', 'bootstrap' properties") : _("Define the 'name', 'peer', 'address', 'gateway' properties"))
            : (m_bootItem ? _("Define the 'name', 'address', 'gateway', 'bootstrap' properties") : _("Define the 'name', 'address', 'gateway' properties"));
        CMessageDialog dialog(this, message, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
        dialog.ShowModal();
    }
    else
        event.Skip();
}
