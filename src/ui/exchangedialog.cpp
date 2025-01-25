#include "exchangedialog.h"
#include "messagedialog.h"

CImportPage::CImportPage(const wxString& pier, const WebPier::Context::ServiceList& forImport, wxWindow* parent) 
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxEmptyString)
{
    this->SetLabel(_("Select services to import"));

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    wxStaticBoxSizer* pierSizer;
    pierSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, pier ), wxVERTICAL );

    wxBoxSizer* listSizer;
    listSizer = new wxBoxSizer( wxHORIZONTAL );

    wxArrayString choices;
    for (const auto& item : forImport)
    {
        m_import.push_back(item.second);
        choices.Add(item.second->Name);
    }

    m_serviceList = new wxCheckListBox( pierSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,10 ), choices, wxLB_SINGLE );
    listSizer->Add( m_serviceList, 0, wxALL|wxEXPAND, 5 );

    wxBoxSizer* tableSizer;
    tableSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer* serviceSizer;
    serviceSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
    serviceSizer->AddGrowableCol( 1 );
    serviceSizer->SetFlexibleDirection( wxHORIZONTAL );
    serviceSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_nameLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Service"), wxDefaultPosition, wxDefaultSize, 0 );
    m_nameLabel->Wrap( -1 );
    serviceSizer->Add( m_nameLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_nameValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_nameValue->Wrap( -1 );
    serviceSizer->Add( m_nameValue, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_addressLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Address"), wxDefaultPosition, wxDefaultSize, 0 );
    m_addressLabel->Wrap( -1 );
    serviceSizer->Add( m_addressLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_addressCtrl = new wxTextCtrl( pierSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_addressCtrl->SetToolTip( _("Local endpoint to map remote service") );
    serviceSizer->Add( m_addressCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_gatewayLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Gateway"), wxDefaultPosition, wxDefaultSize, 0 );
    m_gatewayLabel->Wrap( -1 );
    serviceSizer->Add( m_gatewayLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_gatewayCtrl = new wxTextCtrl( pierSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_gatewayCtrl->SetToolTip( _("Local endpoint to to bind tunnel") );
    serviceSizer->Add( m_gatewayCtrl, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

    m_startLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Autostart"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startLabel->Wrap( -1 );
    serviceSizer->Add( m_startLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_startCtrl = new wxCheckBox( pierSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    serviceSizer->Add( m_startCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_obscureLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Obscure"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureLabel->Wrap( -1 );
    serviceSizer->Add( m_obscureLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_obscureValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("yes"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureValue->Wrap( -1 );
    serviceSizer->Add( m_obscureValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_rendLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Rendezvous"), wxDefaultPosition, wxDefaultSize, 0 );
    m_rendLabel->Wrap( -1 );
    serviceSizer->Add( m_rendLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_rendValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_rendValue->Wrap( -1 );
    serviceSizer->Add( m_rendValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    tableSizer->Add( serviceSizer, 1, wxALL|wxEXPAND, 5 );

    listSizer->Add( tableSizer, 1, wxEXPAND, 5 );
    pierSizer->Add( listSizer, 1, wxALL|wxEXPAND, 5 );
    mainSizer->Add( pierSizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    m_serviceList->Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(CImportPage::onListItemSelected), NULL, this);
    m_addressCtrl->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CImportPage::onServiceCtrlKillFocus), NULL, this);
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
    delete m_gatewayLabel;
    delete m_gatewayCtrl;
    delete m_startLabel;
    delete m_startCtrl;
    delete m_obscureLabel;
    delete m_obscureValue;
    delete m_rendLabel;
    delete m_rendValue;
}

void CImportPage::populate(int line)
{
    if (line == wxNOT_FOUND)
        return;

    const auto& service = m_import[line];

    m_nameValue->SetLabel(service->Name);
    m_addressCtrl->SetValue(service->Address);
    m_gatewayCtrl->SetValue(service->Gateway);
    m_startCtrl->SetValue(service->Autostart);
    m_obscureValue->SetLabel(service->Obscure ? _("yes") : _("no"));
    m_rendValue->SetLabel(service->Rendezvous.IsEmpty() ? _("Email") : _("DHT"));

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
        if (m_import[index]->Address.IsEmpty())
        {
            CMessageDialog dialog(this, _("Define the 'service' properties for all checked services"), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
            dialog.ShowModal();
            return false;
        }
    }
    return true;
}

WebPier::Context::ServiceList CImportPage::GetImport() const
{
    wxArrayInt checked;
    m_serviceList->GetCheckedItems(checked);

    WebPier::Context::ServiceList imports;
    for(auto index : checked)
    {
        WebPier::Context::ServicePtr next = m_import.at(index);
        imports[wxUIntPtr(next.get())] = next;
    }

    return imports;
}

CExportPage::CExportPage(const wxString& pier, const WebPier::Context::ServiceList& forExport, wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, wxEmptyString)
{
    this->SetLabel(_("Select services to export"));

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    mainSizer->SetMinSize( wxSize( 400,-1 ) );
    wxStaticBoxSizer* pierSizer;
    pierSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, pier ), wxVERTICAL );

    wxBoxSizer* listSizer;
    listSizer = new wxBoxSizer( wxHORIZONTAL );

    wxArrayString choices;
    for (const auto& item : forExport)
    {
        m_export.push_back(item.second);
        choices.Add(item.second->Name);
    }

    m_serviceList = new wxCheckListBox( pierSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize( -1,10 ), choices, wxLB_SINGLE );

    for (size_t i = 0; i < m_export.size(); ++i)
    {
        if (m_export[i]->HasPier(pier))
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

    m_nameLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Service"), wxDefaultPosition, wxDefaultSize, 0 );
    m_nameLabel->Wrap( -1 );
    serviceSizer->Add( m_nameLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_nameValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_nameValue->Wrap( -1 );
    serviceSizer->Add( m_nameValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_addressLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Address"), wxDefaultPosition, wxDefaultSize, 0 );
    m_addressLabel->Wrap( -1 );
    serviceSizer->Add( m_addressLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_addressValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("0.0.0.0:0"), wxDefaultPosition, wxDefaultSize, 0 );
    m_addressValue->Wrap( -1 );
    serviceSizer->Add( m_addressValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_gatewayLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Gateway"), wxDefaultPosition, wxDefaultSize, 0 );
    m_gatewayLabel->Wrap( -1 );
    serviceSizer->Add( m_gatewayLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_gatewayValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("0.0.0.0:0"), wxDefaultPosition, wxDefaultSize, 0 );
    m_gatewayValue->Wrap( -1 );
    serviceSizer->Add( m_gatewayValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_startLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Autostart"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startLabel->Wrap( -1 );
    serviceSizer->Add( m_startLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_startValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("no"), wxDefaultPosition, wxDefaultSize, 0 );
    m_startValue->Wrap( -1 );
    serviceSizer->Add( m_startValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_obscureLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Obscure"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureLabel->Wrap( -1 );
    serviceSizer->Add( m_obscureLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_obscureValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("yes"), wxDefaultPosition, wxDefaultSize, 0 );
    m_obscureValue->Wrap( -1 );
    serviceSizer->Add( m_obscureValue, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_rendLabel = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _("Rendezvous"), wxDefaultPosition, wxDefaultSize, 0 );
    m_rendLabel->Wrap( -1 );
    serviceSizer->Add( m_rendLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_rendValue = new wxStaticText( pierSizer->GetStaticBox(), wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, 0 );
    m_rendValue->Wrap( -1 );
    serviceSizer->Add( m_rendValue, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

    tableSizer->Add( serviceSizer, 1, wxEXPAND|wxALL, 5 );

    listSizer->Add( tableSizer, 1, wxEXPAND, 5 );
    pierSizer->Add( listSizer, 1, wxALL|wxEXPAND, 5 );
    mainSizer->Add( pierSizer, 1, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

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
    delete m_gatewayLabel;
    delete m_gatewayValue;
    delete m_startLabel;
    delete m_startValue;
    delete m_obscureLabel;
    delete m_obscureValue;
    delete m_rendLabel;
    delete m_rendValue;
}

void CExportPage::populate(int line)
{
    if (line == wxNOT_FOUND)
        return;

    const auto& service = m_export[line];

    m_nameValue->SetLabel(service->Name);
    m_addressValue->SetLabel(service->Address);
    m_gatewayValue->SetLabel(service->Gateway);
    m_startValue->SetLabel(service->Autostart ? _("yes") : _("no"));
    m_obscureValue->SetLabel(service->Obscure ? _("yes") : _("no"));
    m_rendValue->SetLabel(service->Rendezvous.IsEmpty() ? _("Email") : _("DHT"));

    this->Layout();
}

void CExportPage::onListItemSelected(wxCommandEvent& event)
{
    populate(event.GetSelection());
    event.Skip();
}

WebPier::Context::ServiceList CExportPage::GetExport() const
{
    wxArrayInt checked;
    m_serviceList->GetCheckedItems(checked);

    WebPier::Context::ServiceList exports;
    for(auto index : checked)
    {
        WebPier::Context::ServicePtr next = m_export.at(index);
        exports[wxUIntPtr(next.get())] = next;
    }

    return exports;
}

CExchangeDialog::CExchangeDialog(const wxString& pier, const WebPier::Context::ServiceList& forImport, const WebPier::Context::ServiceList& forExport, wxWindow* parent) 
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

    m_reply = new wxCheckBox( this, wxID_ANY, _("Make a counter offer"), wxDefaultPosition, wxDefaultSize, 0 );
    m_reply->SetValue(true);
    footSizer->Add( m_reply, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    m_reply->Hide();

    footSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    m_back = new wxButton( this, wxID_BACKWARD, _("Back"), wxDefaultPosition, wxDefaultSize, 0 );
#ifndef WIN32
    m_back->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_BACK), wxASCII_STR(wxART_BUTTON) ) );
#endif
    m_back->Disable();
    footSizer->Add( m_back, 0, wxALL, 5 );

    m_next = new wxButton( this, wxID_FORWARD, _("Next"), wxDefaultPosition, wxDefaultSize, 0 );
#ifndef WIN32
    m_next->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_FORWARD), wxASCII_STR(wxART_BUTTON) ) );
    m_next->SetBitmapPosition( wxRIGHT );
#endif
    footSizer->Add( m_next, 0, wxALL, 5 );

    m_cancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
#ifndef WIN32
    m_cancel->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_CROSS_MARK), wxASCII_STR(wxART_BUTTON) ) );
#endif
    footSizer->Add( m_cancel, 0, wxALL, 5 );

    mainSizer->Add( footSizer, 0, wxEXPAND, 0 );

    this->SetSizer( mainSizer );
    this->Layout();
    mainSizer->Fit( this );

    this->Centre( wxBOTH );

    m_back->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CExchangeDialog::onBackButtonClick ), NULL, this );
    m_next->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CExchangeDialog::onNextButtonClick ), NULL, this );
}

CExchangeDialog::CExchangeDialog(const wxString& host, const WebPier::Context::ServiceList& forExport, wxWindow* parent)
    : CExchangeDialog(host, WebPier::Context::ServiceList(), forExport, parent)
{
    m_back->Hide();
    m_purge->Hide();
    m_importPage->Hide();
    m_exportPage->Show();
    m_next->SetLabel(_("OK"));
#ifndef WIN32
    m_next->SetBitmap(wxArtProvider::GetBitmap(wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON)));
    m_next->SetBitmapPosition(wxLEFT);
#endif
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
#ifndef WIN32
    m_next->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_GO_FORWARD), wxASCII_STR(wxART_BUTTON) ) );
    m_next->SetBitmapPosition( wxRIGHT );
#endif

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
#ifndef WIN32
            m_next->SetBitmap( wxArtProvider::GetBitmap( wxASCII_STR(wxART_TICK_MARK), wxASCII_STR(wxART_BUTTON) ) );
            m_next->SetBitmapPosition( wxLEFT );
#endif
            this->SetTitle(m_exportPage->GetLabel());
            this->Layout();
        }
    }
    event.Skip();
}
