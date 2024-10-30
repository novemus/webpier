#include "importdialog.h"
#include "messagedialog.h"

///////////////////////////////////////////////////////////////////////////

CImportDialog::CImportDialog(const wxString& host, const wxVector<WebPier::Service>& remotes,  wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
    : wxDialog( parent, id, title, pos, size, style )
    , m_remotes(remotes)
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    wxStaticBoxSizer* peerSizer;
    peerSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, host ), wxVERTICAL );

    wxBoxSizer* listSizer;
    listSizer = new wxBoxSizer( wxHORIZONTAL );

    wxArrayString choices;
    for (const auto& service : m_remotes)
        choices.Add(service.GetId());

    m_serviceList = new wxCheckListBox( peerSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,10 ), choices, wxLB_SINGLE );
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
    serviceSizer->Add( m_idLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_idValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_idValue->Wrap( -1 );
    serviceSizer->Add( m_idValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_serviceLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Service"), wxDefaultPosition, wxDefaultSize, 0 );
    m_serviceLabel->Wrap( -1 );
    m_serviceLabel->SetToolTip( _("Local endpoint to map remote service") );

    serviceSizer->Add( m_serviceLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_serviceCtrl = new wxTextCtrl( peerSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_serviceCtrl->SetToolTip( _("Local endpoint to map remote service") );

    serviceSizer->Add( m_serviceCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_gateLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Gateway"), wxDefaultPosition, wxDefaultSize, 0 );
    m_gateLabel->Wrap( -1 );
    m_gateLabel->SetToolTip( _("Local endpoint to bind wormhole tunnel") );

    serviceSizer->Add( m_gateLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_gateCtrl = new wxTextCtrl( peerSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_gateCtrl->SetToolTip( _("Local endpoint to bind wormhole tunnel") );

    serviceSizer->Add( m_gateCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_startLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Autostart"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startLabel->Wrap( -1 );
    serviceSizer->Add( m_startLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_startCtrl = new wxCheckBox( peerSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    serviceSizer->Add( m_startCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_obscureLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Obscure"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureLabel->Wrap( -1 );
    serviceSizer->Add( m_obscureLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_obscureValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("yes"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureValue->Wrap( -1 );
    serviceSizer->Add( m_obscureValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );
    tableSizer->Add( serviceSizer, 1, wxALL|wxEXPAND, 5 );

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

    wxBoxSizer* footSizer;
    footSizer = new wxBoxSizer( wxHORIZONTAL );

    m_backCtrl = new wxCheckBox( this, wxID_ANY, _("Make back advertising"), wxDefaultPosition, wxDefaultSize, 0 );
    m_backCtrl->SetValue(true);
    footSizer->Add( m_backCtrl, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );

    wxStdDialogButtonSizer* sdbSizer;
    sdbSizer = new wxStdDialogButtonSizer();
    m_ok = new wxButton( this, wxID_OK );
    sdbSizer->AddButton( m_ok );
    m_cancel = new wxButton( this, wxID_CANCEL );
    sdbSizer->AddButton( m_cancel );
    sdbSizer->Realize();

    footSizer->Add( sdbSizer, 1, wxBOTTOM, 10 );
    mainSizer->Add( footSizer, 0, wxEXPAND, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    m_serviceList->Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(CImportDialog::onListItemSelected), NULL, this);
    m_serviceCtrl->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CImportDialog::onServiceCtrlKillFocus), NULL, this);
    m_gateCtrl->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CImportDialog::onGatewayCtrlKillFocus), NULL, this);
    m_startCtrl->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CImportDialog::onAutostartCheckBox), NULL, this);
    m_ok->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CImportDialog::onOKButtonClick), NULL, this);

    populate();
}

CImportDialog::~CImportDialog()
{
    delete m_serviceList;
    delete m_idLabel;
    delete m_idValue;
    delete m_serviceLabel;
    delete m_serviceCtrl;
    delete m_gateLabel;
    delete m_gateCtrl;
    delete m_startLabel;
    delete m_startCtrl;
    delete m_obscureLabel;
    delete m_obscureValue;
    delete m_bootLabel;
    delete m_bootValue;
    delete m_netLabel;
    delete m_netValue;
    delete m_backCtrl;
    delete m_ok;
    delete m_cancel;
}

void CImportDialog::populate()
{
    if (m_remotes.empty())
        return;

    int line = m_serviceList->GetSelection();
    if (line == wxNOT_FOUND)
    {
        line = 0;
        m_serviceList->SetSelection(line);
    }

    const auto& service = m_remotes[line];

    m_idValue->SetLabel(service.GetId());
    m_serviceCtrl->SetValue(service.GetService());
    m_gateCtrl->SetValue(service.GetGateway());
    m_startCtrl->SetValue(service.IsAutostart());
    m_obscureValue->SetLabel(service.IsObscure() ? _("yes") : _("no"));
    m_bootValue->SetLabel(service.GetDhtBootstrap());
    m_netValue->SetLabel(wxString::Format(wxT("%d"), (int)service.GetDhtNetwork()));

    this->Layout();
}

void CImportDialog::onListItemSelected(wxCommandEvent& event)
{
    populate();
    event.Skip();
}

void CImportDialog::onServiceCtrlKillFocus(wxFocusEvent& event)
{
    if (m_remotes.empty())
        return;

    int line = m_serviceList->GetSelection();
    if (line == wxNOT_FOUND)
    {
        line = 0;
        m_serviceList->SetSelection(line);
    }

    m_remotes[line].SetService(m_serviceCtrl->GetValue());
    event.Skip();
}

void CImportDialog::onGatewayCtrlKillFocus(wxFocusEvent& event)
{
    if (m_remotes.empty())
        return;

    int line = m_serviceList->GetSelection();
    if (line == wxNOT_FOUND)
    {
        line = 0;
        m_serviceList->SetSelection(line);
    }

    m_remotes[line].SetGateway(m_gateCtrl->GetValue());
    event.Skip();
}

void CImportDialog::onAutostartCheckBox(wxCommandEvent& event)
{
    if (m_remotes.empty())
        return;

    int line = m_serviceList->GetSelection();
    if (line == wxNOT_FOUND)
    {
        line = 0;
        m_serviceList->SetSelection(line);
    }

    m_remotes[line].SetAutostart(m_startCtrl->IsChecked());
    event.Skip();
}

void CImportDialog::onOKButtonClick(wxCommandEvent& event)
{
    wxArrayInt checked;
    m_serviceList->GetCheckedItems(checked);

    for(auto index : checked)
    {
        if (m_remotes[index].GetService().IsEmpty() || m_remotes[index].GetGateway().IsEmpty())
        {
            CMessageDialog dialog(this, _("Define the 'service' and 'gateway' properties for all checked services"), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
            dialog.ShowModal();
            return;
        }
    }

    event.Skip();
}

wxVector<WebPier::Service> CImportDialog::GetImport() const
{
    wxArrayInt checked;
    m_serviceList->GetCheckedItems(checked);

    wxVector<WebPier::Service> exports;
    for(auto index : checked)
        exports.push_back(m_remotes[index]);

    return exports;
}
