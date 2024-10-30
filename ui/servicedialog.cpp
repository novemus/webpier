#include "servicedialog.h"
#include "messagedialog.h"
#include <wx/valnum.h>

CServiceDialog::CServiceDialog(WebPier::Service* service, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style), m_service(service)
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
    m_idItem = m_propGrid->Append( new wxStringProperty( _("Id"), wxPG_LABEL, m_service->GetId() ) );
    if (m_service->IsLocal())
    {
        m_peerItem = m_propGrid->Append( new wxMultiChoiceProperty( _("Peer"), wxPG_LABEL, peerChoice, m_service->GetPees()));
    }
    else
    {
        int value = 0;
        for(auto& peer : peerChoice)
        {
            if (peer == m_service->GetPeer())
                break;
            ++value;
        }
        m_peerItem = m_propGrid->Append( new wxEnumProperty( _("Peer"), wxPG_LABEL, peerChoice, wxArrayInt(), value ) );
    }

    m_servItem = m_propGrid->Append( new wxStringProperty( _("Service"), wxPG_LABEL, m_service->GetService() ) );
    m_gateItem = m_propGrid->Append( new wxStringProperty( _("Gateway"), wxPG_LABEL, m_service->GetGateway() ) );
    m_startItem = m_propGrid->Append( new wxBoolProperty( _("Autostart"), wxPG_LABEL, m_service->IsAutostart() ) );
    m_obsItem = m_propGrid->Append( new wxBoolProperty( _("Obscure"), wxPG_LABEL, m_service->IsObscure() ) );
    m_rendItem = m_propGrid->Append( new wxEnumProperty( _("Rendezvous"), wxPG_LABEL, rendChoice, wxArrayInt(), m_service->IsDhtRendezvous() ? 0 : 1 ) );

    if (m_service->IsDhtRendezvous())
    {
        m_bootItem = m_rendItem->InsertChild( 0, new wxStringProperty( _("Bootstrap"), wxPG_LABEL, m_service->GetDhtBootstrap() ) );
        m_netItem = m_rendItem->InsertChild( 1, new wxUIntProperty( _("Network"), wxPG_LABEL, m_service->GetDhtNetwork() ) );
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
    m_cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CServiceDialog::onCancelButtonClick ), NULL, this );
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
            m_service->UseDhtRendezvous();
            m_bootItem = m_rendItem->InsertChild( 0, new wxStringProperty( _("Bootstrap"), wxPG_LABEL, m_service->GetDhtBootstrap() ) );
            m_netItem = m_rendItem->InsertChild( 1, new wxUIntProperty( _("Network"), wxPG_LABEL, m_service->GetDhtNetwork() ) );
        }
        else
        {
            m_service->UseEmailRendezvous();
            m_rendItem->DeleteChildren();
            m_bootItem = nullptr;
            m_netItem = nullptr;
        }
    }
    else if (prop == m_idItem)
    {
        m_service->SetId(m_idItem->GetValueAsString());
    }
    else if (prop == m_peerItem)
    {
        if (m_service->IsLocal())
            m_service->SetPeers(m_peerItem->GetValue().GetArrayString());
        else
            m_service->SetPeer(m_peerItem->GetValueAsString());
    }
    else if (prop == m_servItem)
    {
        m_service->SetService(m_servItem->GetValueAsString());
    }
    else if (prop == m_gateItem)
    {
        m_service->SetGateway(m_gateItem->GetValueAsString());
    }
    else if (prop == m_obsItem)
    {
        m_service->SetObscure(m_obsItem->GetValue().GetBool());
    }
    else if (prop == m_startItem)
    {
        m_service->SetAutostart(m_startItem->GetValue().GetBool());
    }
    else if (prop == m_bootItem && m_bootItem)
    {
        auto bootstrap = m_bootItem->GetValueAsString();
        if (bootstrap.IsEmpty())
            m_bootItem->SetValueFromString(m_service->GetDhtBootstrap());
        else
            m_service->SetDhtBootstrap(bootstrap);
    }
    else if (prop == m_netItem && m_netItem)
    {
        auto network = m_netItem->GetValueAsString();
        if (network.IsEmpty())
            m_netItem->SetValueFromInt(0);
        else
            m_service->SetDhtNetwork(m_netItem->GetValue().GetULongLong().GetLo());
    }
    this->Layout();
}

void CServiceDialog::onOKButtonClick( wxCommandEvent& event )
{
    if (m_service->GetId().IsEmpty() || m_service->GetService().IsEmpty() || m_service->GetGateway().IsEmpty() || (m_service->IsRemote() && m_service->GetPeer().IsEmpty()))
    {
        wxString message = m_service->IsRemote() 
            ? _("Define the 'id', 'service', 'gateway' and 'peer' properties") 
            : _("Define the 'id', 'service' and 'gateway' properties");
        CMessageDialog dialog(this, message, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
        dialog.ShowModal();
    }
    else
        event.Skip();
}

void CServiceDialog::onCancelButtonClick( wxCommandEvent& event )
{
    m_service->Revert();
    event.Skip(); 
}
