#include "exchangedialog.h"
#include "messagedialog.h"

CImportPage::CImportPage(const wxString& pier, const WebPier::ServiceList& forImport, wxWindow* parent) 
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxEmptyString)
{
    this->SetLabel(_("Select services to import"));

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    wxStaticBoxSizer* peerSizer;
    peerSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, pier ), wxVERTICAL );

    wxBoxSizer* listSizer;
    listSizer = new wxBoxSizer( wxHORIZONTAL );

    wxArrayString choices;
    for (const auto& item : forImport)
    {
        m_import.push_back(item.second);
        choices.Add(item.second->Name);
    }

    m_serviceList = new wxCheckListBox( peerSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,10 ), choices, wxLB_SINGLE );
    listSizer->Add( m_serviceList, 0, wxALL|wxEXPAND, 5 );

    wxBoxSizer* tableSizer;
    tableSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* serviceSizer;
    serviceSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
    serviceSizer->AddGrowableCol( 1 );
    serviceSizer->SetFlexibleDirection( wxHORIZONTAL );
    serviceSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_nameLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Service"), wxDefaultPosition, wxDefaultSize, 0 );
    m_nameLabel->Wrap( -1 );
    serviceSizer->Add( m_nameLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_nameValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_nameValue->Wrap( -1 );
    serviceSizer->Add( m_nameValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_addressLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Address"), wxDefaultPosition, wxDefaultSize, 0 );
    m_addressLabel->Wrap( -1 );
    m_addressLabel->SetToolTip( _("Local endpoint to map remote service") );

    serviceSizer->Add( m_addressLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_addressCtrl = new wxTextCtrl( peerSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_addressCtrl->SetToolTip( _("Local endpoint to map remote service") );

    serviceSizer->Add( m_addressCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

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
    mainSizer->Add( peerSizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    m_serviceList->Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(CImportPage::onListItemSelected), NULL, this);
    m_addressCtrl->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CImportPage::onServiceCtrlKillFocus), NULL, this);
    m_gateCtrl->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CImportPage::onGatewayCtrlKillFocus), NULL, this);
    m_startCtrl->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(CImportPage::onAutostartCheckBox), NULL, this);

    if (!m_import.empty())
    {
        m_serviceList->SetSelection(0);
        populate(0);
    }
}

CImportPage::~CImportPage()
{
    delete m_serviceList;
    delete m_nameLabel;
    delete m_nameValue;
    delete m_addressLabel;
    delete m_addressCtrl;
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
}

void CImportPage::populate(int line)
{
    if (line == wxNOT_FOUND)
        return;

    const auto& service = m_import[line];

    m_nameValue->SetLabel(service->Name);
    m_addressCtrl->SetValue(service->Address);
    m_gateCtrl->SetValue(service->Gateway);
    m_startCtrl->SetValue(service->Autostart);
    m_obscureValue->SetLabel(service->Obscure ? _("yes") : _("no"));
    m_bootValue->SetLabel(service->DhtBootstrap);
    m_netValue->SetLabel(wxString::Format(wxT("%d"), (int)service->DhtNetwork));

    this->Layout();
}

void CImportPage::onListItemSelected(wxCommandEvent& event)
{
    populate(event.GetSelection());
    event.Skip();
}

void CImportPage::onServiceCtrlKillFocus(wxFocusEvent& event)
{
    if (m_import.empty())
        return;

    int line = m_serviceList->GetSelection();
    if (line == wxNOT_FOUND)
    {
        line = 0;
        m_serviceList->SetSelection(line);
    }

    m_import[line]->Address = m_addressCtrl->GetValue();
    event.Skip();
}

void CImportPage::onGatewayCtrlKillFocus(wxFocusEvent& event)
{
    if (m_import.empty())
        return;

    int line = m_serviceList->GetSelection();
    if (line == wxNOT_FOUND)
    {
        line = 0;
        m_serviceList->SetSelection(line);
    }

    m_import[line]->Gateway = m_gateCtrl->GetValue();
    event.Skip();
}

void CImportPage::onAutostartCheckBox(wxCommandEvent& event)
{
    if (m_import.empty())
        return;

    int line = m_serviceList->GetSelection();
    if (line == wxNOT_FOUND)
    {
        line = 0;
        m_serviceList->SetSelection(line);
    }

    m_import[line]->Autostart = m_startCtrl->IsChecked();
    event.Skip();
}

bool CImportPage::ValidateData()
{
    wxArrayInt checked;
    m_serviceList->GetCheckedItems(checked);

    for(auto index : checked)
    {
        if (m_import[index]->Address.IsEmpty() || m_import[index]->Gateway.IsEmpty())
        {
            CMessageDialog dialog(this, _("Define the 'service' and 'gateway' properties for all checked services"), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
            dialog.ShowModal();
            return false;
        }
    }
    return true;
}

WebPier::ServiceList CImportPage::GetImport() const
{
    wxArrayInt checked;
    m_serviceList->GetCheckedItems(checked);

    WebPier::ServiceList imports;
    for(auto index : checked)
    {
        WebPier::ServicePtr next = m_import.at(index);
        imports[wxUIntPtr(next.get())] = next;
    }

    return imports;
}

CExportPage::CExportPage(const wxString& pier, const WebPier::ServiceList& forExport, wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxEmptyString)
{
    this->SetLabel(_("Select services to export"));

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    wxStaticBoxSizer* peerSizer;
    peerSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, pier ), wxVERTICAL );

    wxBoxSizer* listSizer;
    listSizer = new wxBoxSizer( wxHORIZONTAL );

    wxArrayString choices;
    for (const auto& item : forExport)
    {
        m_export.push_back(item.second);
        choices.Add(item.second->Name);
    }

    m_serviceList = new wxCheckListBox( peerSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,10 ), choices, wxLB_SINGLE );

    for (size_t i = 0; i < m_export.size(); ++i)
    {
        if (m_export[i]->HasPeer(pier))
            m_serviceList->Check(i);
    }

    listSizer->Add( m_serviceList, 0, wxALL|wxEXPAND, 5 );

    wxBoxSizer* tableSizer;
    tableSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* serviceSizer;
    serviceSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
    serviceSizer->AddGrowableCol( 1 );
    serviceSizer->SetFlexibleDirection( wxHORIZONTAL );
    serviceSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_nameLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Service"), wxDefaultPosition, wxDefaultSize, 0 );
    m_nameLabel->Wrap( -1 );
    serviceSizer->Add( m_nameLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_nameValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_nameValue->Wrap( -1 );
    serviceSizer->Add( m_nameValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_addressLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Address"), wxDefaultPosition, wxDefaultSize, 0 );
    m_addressLabel->Wrap( -1 );
    m_addressLabel->SetToolTip( _("Endpoint of the local service") );

    serviceSizer->Add( m_addressLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_addressValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("0.0.0.0:0"), wxDefaultPosition, wxDefaultSize, 0 );
    m_addressValue->Wrap( -1 );
    serviceSizer->Add( m_addressValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_gateLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Gateway"), wxDefaultPosition, wxDefaultSize, 0 );
    m_gateLabel->Wrap( -1 );
    m_gateLabel->SetToolTip( _("Local endpoint to bind wormhole tunnel") );

    serviceSizer->Add( m_gateLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_gateValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("0.0.0.0:0"), wxDefaultPosition, wxDefaultSize, 0 );
    m_gateValue->Wrap( -1 );
    serviceSizer->Add( m_gateValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_startLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Autostart"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startLabel->Wrap( -1 );
    serviceSizer->Add( m_startLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_startValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("no"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startValue->Wrap( -1 );
    serviceSizer->Add( m_startValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_obscureLabel = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("Obscure"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureLabel->Wrap( -1 );
    serviceSizer->Add( m_obscureLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_obscureValue = new wxStaticText( peerSizer->GetStaticBox(), wxID_ANY, _("yes"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureValue->Wrap( -1 );
    serviceSizer->Add( m_obscureValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

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

    m_bootValue = new wxStaticText( rendSizer->GetStaticBox(), wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0 );
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
    mainSizer->Add( peerSizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    m_serviceList->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( CExportPage::onListItemSelected ), NULL, this );

    if (!m_export.empty())
    {
        m_serviceList->SetSelection(0);
        populate(0);
    }
}

CExportPage::~CExportPage()
{
    delete m_serviceList;
    delete m_nameLabel;
    delete m_nameValue;
    delete m_addressLabel;
    delete m_addressValue;
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
}

void CExportPage::populate(int line)
{
    if (line == wxNOT_FOUND)
        return;

    const auto& service = m_export[line];

    m_nameValue->SetLabel(service->Name);
    m_addressValue->SetLabel(service->Address);
    m_gateValue->SetLabel(service->Gateway);
    m_startValue->SetLabel(service->Autostart ? _("yes") : _("no"));
    m_obscureValue->SetLabel(service->Obscure ? _("yes") : _("no"));
    m_bootValue->SetLabel(service->DhtBootstrap);
    m_netValue->SetLabel(wxString::Format(wxT("%d"), (int)service->DhtNetwork));

    this->Layout();
}

void CExportPage::onListItemSelected(wxCommandEvent& event)
{
    populate(event.GetSelection());
    event.Skip();
}

WebPier::ServiceList CExportPage::GetExport() const
{
    wxArrayInt checked;
    m_serviceList->GetCheckedItems(checked);

    WebPier::ServiceList exports;
    for(auto index : checked)
    {
        WebPier::ServicePtr next = m_export.at(index);
        exports[wxUIntPtr(next.get())] = next;
    }

    return exports;
}

CExchangeDialog::CExchangeDialog(const wxString& pier, const WebPier::ServiceList& forImport, const WebPier::ServiceList& forExport, wxWindow* parent) 
    : wxDialog( parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    mainSizer->SetMinSize( wxSize( 500,-1 ) );
    m_importPage = new CImportPage( pier, forImport, this);
    mainSizer->Add( m_importPage, 1, wxEXPAND, 0 );
    this->SetTitle(m_importPage->GetLabel());

    m_exportPage = new CExportPage( pier, forExport, this);
    mainSizer->Add( m_exportPage, 1, wxEXPAND, 0 );
    m_exportPage->Hide();

    wxBoxSizer* footSizer;
    footSizer = new wxBoxSizer( wxHORIZONTAL );

    m_purge = new wxCheckBox( this, wxID_ANY, _("Purge existing services"), wxDefaultPosition, wxDefaultSize, 0 );
    m_purge->SetValue(false);
    footSizer->Add( m_purge, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_reply = new wxCheckBox( this, wxID_ANY, _("Make a reply advertisement"), wxDefaultPosition, wxDefaultSize, 0 );
    m_reply->SetValue(true);
    footSizer->Add( m_reply, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    m_reply->Hide();

    footSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    m_back = new wxButton( this, wxID_BACKWARD, _("Back"), wxDefaultPosition, wxDefaultSize, 0 );
    m_back->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_BACK), wxASCII_STR(wxART_BUTTON) ) );
    m_back->Disable();
    footSizer->Add( m_back, 0, wxALL, 5 );

    m_next = new wxButton( this, wxID_FORWARD, _("Next"), wxDefaultPosition, wxDefaultSize, 0 );
    m_next->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_FORWARD), wxASCII_STR(wxART_BUTTON) ) );
    m_next->SetBitmapPosition( wxRIGHT );
    footSizer->Add( m_next, 0, wxALL, 5 );

    m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_cancel->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON) ) );
    footSizer->Add( m_cancel, 0, wxALL, 5 );

    mainSizer->Add( footSizer, 0, wxEXPAND, 0 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    m_back->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CExchangeDialog::onBackButtonClick ), NULL, this );
    m_next->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CExchangeDialog::onNextButtonClick ), NULL, this );
}

CExchangeDialog::CExchangeDialog(const wxString& host, const WebPier::ServiceList& forExport, wxWindow* parent)
    : CExchangeDialog(host, WebPier::ServiceList(), forExport, parent)
{
    m_back->Hide();
    m_purge->Hide();
    m_importPage->Hide();
    m_exportPage->Show();
    m_next->SetLabel(_("OK"));
    m_next->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON)));
    m_next->SetBitmapPosition(wxLEFT);

    this->SetTitle(m_exportPage->GetLabel());
    this->Layout();
}

CExchangeDialog::~CExchangeDialog()
{
    delete m_importPage;
    delete m_exportPage;
    delete m_purge;
    delete m_reply;
    delete m_cancel;
    delete m_back;
    delete m_next;
}

void CExchangeDialog::onBackButtonClick(wxCommandEvent& event)
{
    m_back->Disable();
    m_exportPage->Hide();
    m_importPage->Show();
    m_reply->Hide();
    m_purge->Show();

    m_next->SetLabel(_("Next"));
    m_next->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_FORWARD), wxASCII_STR(wxART_BUTTON) ) );
    m_next->SetBitmapPosition( wxRIGHT );

    this->SetTitle(m_importPage->GetLabel());
    this->Layout();

    event.Skip();
}

void CExchangeDialog::onNextButtonClick(wxCommandEvent& event)
{
    if (m_exportPage->IsShown())
    {
        this->EndModal(wxID_OK);
    }
    else
    {
        if (m_importPage->ValidateData())
        {
            m_importPage->Hide();
            m_exportPage->Show();
            m_purge->Hide();
            m_reply->Show();
            m_back->Enable();

            m_next->SetLabel(_("OK"));
            m_next->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON) ) );
            m_next->SetBitmapPosition( wxLEFT );

            this->SetTitle(m_exportPage->GetLabel());
            this->Layout();
        }
    }
    event.Skip();
}
