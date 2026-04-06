#include <ui/servicedialog.h>
#include <ui/messagedialog.h>
#include <ui/logo.h>
#include <wx/valnum.h>

CServiceDialog::CServiceDialog(WebPier::Context::ConfigPtr config, WebPier::Context::ServicePtr service, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style)
    , m_config(config)
    , m_service(service)
{
    this->SetIcon(::GetAppIconBundle().GetIcon());
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    wxArrayString rendChoice;
    rendChoice.Add(wxT("Email"));
    rendChoice.Add(wxT("DHT"));

    wxArrayString protoChoice;
    protoChoice.Add(ToString(WebPier::Context::Service::Any));
    protoChoice.Add(ToString(WebPier::Context::Service::UDP));
    protoChoice.Add(ToString(WebPier::Context::Service::TCP));
    protoChoice.Add(ToString(WebPier::Context::Service::SSL));

    wxArrayString schemaChoice;
    schemaChoice.Add(ToString(WebPier::Context::Service::Either));
    schemaChoice.Add(ToString(WebPier::Context::Service::Client));
    schemaChoice.Add(ToString(WebPier::Context::Service::Server));
    schemaChoice.Add(ToString(WebPier::Context::Service::Mutual));

    auto pierChoice = WebPier::Context::GetPiers();

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    m_propGrid = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_BOLD_MODIFIED|wxPG_HIDE_MARGIN);
    m_nameItem = m_propGrid->Append( new wxStringProperty( _("Name"), wxPG_LABEL, m_service->Name ) );
    if (m_service->Local)
    {
        m_pierItem = m_propGrid->Append(new wxMultiChoiceProperty( _("Pier"), wxPG_LABEL, pierChoice, wxSplit(m_service->Pier, ' ')));
    }
    else
    {
        int value = 0;
        for(auto& pier : pierChoice)
        {
            if (pier == m_service->Pier)
                break;
            ++value;
        }
        m_pierItem = m_propGrid->Append( new wxEnumProperty( _("Pier"), wxPG_LABEL, pierChoice, wxArrayInt(), value ) );
    }

    m_addrItem = m_propGrid->Append( new wxStringProperty( _("Address"), wxPG_LABEL, m_service->Address ) );
    m_protoItem = m_propGrid->Append( new wxEnumProperty( _("Tunnel"), wxPG_LABEL, protoChoice, wxArrayInt(), static_cast<int>(m_service->Proto) ) );
    m_gateItem = m_protoItem->InsertChild( 0, new wxStringProperty( _("Gateway"), wxPG_LABEL, m_service->Gateway ) );
	m_roleItem = m_protoItem->InsertChild( 1, new wxEnumProperty( _("Schema"), wxPG_LABEL, schemaChoice, wxArrayInt(),  static_cast<int>(m_service->Role) ) );
    m_startItem = m_protoItem->InsertChild( 2, new wxBoolProperty( _("Autostart"), wxPG_LABEL, m_service->Autostart ) );
    m_obsItem = m_protoItem->InsertChild( 3, new wxBoolProperty( _("Obscure"), wxPG_LABEL, m_service->Obscure ) );
    m_obsItem->Hide(m_service->Proto == WebPier::Context::Service::SSL);
    m_rendItem = m_propGrid->Append( new wxEnumProperty( _("Rendezvous"), wxPG_LABEL, rendChoice, wxArrayInt(), m_service->Rendezvous.IsEmpty() ? 0 : 1 ) );
    m_bootItem = m_rendItem->InsertChild( 0, new wxStringProperty( _("Bootstrap"), wxPG_LABEL, m_service->Rendezvous ) );
    m_bootItem->Hide(m_service->Rendezvous.IsEmpty());

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
    m_propGrid->FitColumns();

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
        m_bootItem->Hide(m_rendItem->GetChoiceSelection() == 0);
    else if (prop == m_protoItem)
        m_obsItem->Hide(m_protoItem->GetChoiceSelection() == 3);

    this->Layout();
}

void CServiceDialog::onOKButtonClick( wxCommandEvent& event )
{
    m_service->Name = m_nameItem->GetValueAsString();
    m_service->Pier = m_service->Local ? wxJoin(m_pierItem->GetValue().GetArrayString(), ' ') : m_pierItem->GetValueAsString();
    m_service->Address = m_addrItem->GetValueAsString();
    m_service->Gateway = m_gateItem->GetValueAsString();
    m_service->Proto = static_cast<WebPier::Context::Service::Protocol>(m_protoItem->GetValue().GetLong());
    m_service->Role = static_cast<WebPier::Context::Service::Schema>(m_roleItem->GetValue().GetLong());
    m_service->Obscure = m_obsItem->GetValue().GetBool();
    m_service->Autostart = m_startItem->GetValue().GetBool();
    m_service->Rendezvous = m_rendItem->GetChoiceSelection() == 1 ? m_bootItem->GetValueAsString() : "";

    if (m_service->Name.IsEmpty() || m_service->Address.IsEmpty() || (!m_service->Local && m_service->Pier.IsEmpty()) || (m_rendItem->GetChoiceSelection() == 1 && m_service->Rendezvous.IsEmpty()))
    {
        wxString message = m_service->Local 
            ? (m_rendItem->GetChoiceSelection() == 1 ? _("Define the 'name', 'pier', 'address', 'gateway', 'bootstrap' properties") : _("Define the 'name', 'pier', 'address', 'gateway' properties"))
            : (m_rendItem->GetChoiceSelection() == 1 ? _("Define the 'name', 'address', 'bootstrap', 'gateway' properties") : _("Define the 'name', 'address', 'gateway' properties"));
        CMessageDialog dialog(this, message, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
        dialog.ShowModal();
    }
    else
        event.Skip();
}
