#include "mainframe.h"
#include "context.h"
#include "servicedialog.h"
#include "settingsdialog.h"
#include "exchangedialog.h"
#include "aboutdialog.h"
#include "messagedialog.h"
#include "logo.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

wxVector<wxVariant> ToVariantList(WebPier::ServicePtr service)
{
    wxVector<wxVariant> data;
    data.push_back(wxVariant(wxDataViewIconText(service->Id, ::GetGreyCircleImage())));
    data.push_back(wxVariant(wxString(service->Peer)));
    data.push_back(wxVariant(wxString(service->Address)));
    data.push_back(wxVariant(wxString(service->Gateway)));
    data.push_back(wxVariant(wxString(service->DhtBootstrap.IsEmpty() ? wxT("Email") : wxT("DHT"))));
    data.push_back(wxVariant(wxString(service->Autostart ? _("yes") : _("no"))));
    return data;
}

CMainFrame::CMainFrame() : wxFrame(nullptr, wxID_ANY, wxT("WebPier"), wxDefaultPosition, wxSize(950, 500), wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );

    wxMenuBar* menubar = new wxMenuBar( 0 );
    wxMenu* fileMenu = new wxMenu();
    wxMenuItem* settingsItem = fileMenu->Append( new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Settings...") ) , wxEmptyString, wxITEM_NORMAL ) );

    fileMenu->AppendSeparator();

    m_exportItem = fileMenu->Append( new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Advertise host...") ) , wxEmptyString, wxITEM_NORMAL ) );
    m_exportItem->Enable(false);

    m_importItem = fileMenu->Append( new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Introduce peer...") ) , wxEmptyString, wxITEM_NORMAL ) );
    m_importItem->Enable(false);

    fileMenu->AppendSeparator();
    wxMenuItem* exitItem = fileMenu->Append( new wxMenuItem( fileMenu, wxID_ANY, wxString( _("E&xit") ) , wxEmptyString, wxITEM_NORMAL ) );

    menubar->Append( fileMenu, _("&File") );

    wxMenu* helpMenu = new wxMenu();
    wxMenuItem* aboutItem = helpMenu->Append( new wxMenuItem( helpMenu, wxID_ANY, wxString( _("&About...") ) , wxEmptyString, wxITEM_NORMAL ) );

    menubar->Append( helpMenu, _("&Help") );

    this->SetMenuBar( menubar );

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* topSizer;
    topSizer = new wxBoxSizer( wxHORIZONTAL );

    m_importBtn = new wxRadioButton( this, wxID_ANY, _("Import"), wxDefaultPosition, wxDefaultSize, 0 );
    m_importBtn->SetValue( true );
    topSizer->Add( m_importBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_exportBtn = new wxRadioButton( this, wxID_ANY, _("Export"), wxDefaultPosition, wxDefaultSize, 0 );
    topSizer->Add( m_exportBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_hostLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL );
	m_hostLabel->Wrap( -1 );
	topSizer->Add( m_hostLabel, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_addBtn = new wxBitmapButton( this, wxID_ANY, wxArtProvider::GetBitmapBundle(wxART_NEW, wxART_BUTTON, wxSize(16, 16)), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
    m_addBtn->Enable(false);
    topSizer->Add( m_addBtn, 0, wxALL, 5 );

    m_editBtn = new wxBitmapButton( this, wxID_ANY, wxArtProvider::GetBitmapBundle(wxART_EDIT, wxART_BUTTON, wxSize(16, 16)), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
    m_editBtn->Enable(false);
    topSizer->Add( m_editBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    m_deleteBtn = new wxBitmapButton( this, wxID_ANY, wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_BUTTON, wxSize(16, 16)), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
    m_deleteBtn->Enable(false);
    topSizer->Add( m_deleteBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    mainSizer->Add( topSizer, 0, wxEXPAND, 5 );

    m_serviceList = new wxDataViewListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES|wxDV_ROW_LINES|wxDV_SINGLE|wxDV_VERT_RULES );
    m_serviceList->AppendIconTextColumn( _("Service"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Peer"), wxDATAVIEW_CELL_INERT, 350, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Address"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Gateway"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Rendezvous"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Autostart"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    mainSizer->Add( m_serviceList, 1, wxALL|wxEXPAND, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    m_statusBar = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );

    this->Centre( wxBOTH );

    fileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onSettingsMenuSelection ), this, settingsItem->GetId());
    fileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onImportMenuSelection ), this, m_importItem->GetId());
    fileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onExportMenuSelection ), this, m_exportItem->GetId());
    fileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onExitMenuSelection ), this, exitItem->GetId());
    helpMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onAboutMenuSelection ), this, aboutItem->GetId());
    m_importBtn->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( CMainFrame::onImportRadioClick ), NULL, this );
    m_exportBtn->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( CMainFrame::onExportRadioClick ), NULL, this );
    m_addBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CMainFrame::onAddServiceButtonClick ), NULL, this );
    m_editBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CMainFrame::onEditServiceButtonClick ), NULL, this );
    m_deleteBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CMainFrame::onDeleteServiceButtonClick ), NULL, this );
    m_serviceList->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler( CMainFrame::onServiceItemContextMenu ), NULL, this );
}

CMainFrame::~CMainFrame()
{
    delete m_importBtn;
    delete m_exportBtn;
    delete m_hostLabel;
    delete m_addBtn;
    delete m_editBtn;
    delete m_deleteBtn;
    delete m_serviceList;
    delete m_statusBar;
}

bool CMainFrame::Show(bool show)
{
    if (show && m_hostLabel->GetLabel().IsEmpty())
        populate();

    return wxFrame::Show(show);
}

void CMainFrame::populate()
{
    try
    {
        m_serviceList->DeleteAllItems();

        m_config = WebPier::GetConfig();
        m_export = WebPier::GetExportServices();
        m_import = WebPier::GetImportServices();

        m_hostLabel->SetLabel(m_config->Host);

        for (auto& item : m_importBtn->GetValue() ? m_import : m_export)
            m_serviceList->AppendItem(ToVariantList(item.second), item.first);

        m_importItem->Enable(true);
        m_exportItem->Enable(true);
        m_addBtn->Enable(true);
        m_editBtn->Enable(true);
        m_deleteBtn->Enable(true);
    }
    catch(const std::exception& ex)
    {
        CMessageDialog dialog(this, _("Can't populate service list: ") + ex.what(), wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
        dialog.ShowModal();

        m_importItem->Enable(false);
        m_exportItem->Enable(false);
        m_addBtn->Enable(false);
        m_editBtn->Enable(false);
        m_deleteBtn->Enable(false);
    }

    this->Layout();
}

void CMainFrame::onExitMenuSelection(wxCommandEvent&)
{
    wxCloseEvent event(wxEVT_CLOSE_WINDOW, GetId());
    event.SetEventObject(this);
    ProcessWindowEvent(event);
}

void CMainFrame::onSettingsMenuSelection(wxCommandEvent& event)
{
    try
    {
        CSettingsDialog dialog(m_config, this);
        if (dialog.ShowModal() == wxID_OK)
        {
            m_config->Store();
            if (m_config->Host != m_hostLabel->GetLabel())
                populate();
        }
    }
    catch (const std::exception &ex)
    {
        CMessageDialog dialog(this, _("Can't setup settings: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onAddServiceButtonClick(wxCommandEvent& event)
{
    WebPier::ServicePtr service = m_exportBtn->GetValue() ? WebPier::CreateExportService() : WebPier::CreateImportService();

    CServiceDialog dialog(m_config, service, this);
    if (dialog.ShowModal() == wxID_OK)
    {
        try
        {
            service->Store();
            if (m_exportBtn->GetValue())
                m_export[wxUIntPtr(service.get())] = service;
            else
                m_import[wxUIntPtr(service.get())] = service;
            m_serviceList->AppendItem(ToVariantList(service), wxUIntPtr(service.get()));
        }
        catch(const std::exception& ex)
        {
            CMessageDialog dialog(this, _("Can't add service: ") + ex.what(), wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
            dialog.ShowModal();
        }
    }
}

void CMainFrame::onEditServiceButtonClick(wxCommandEvent& event)
{
    if (!m_serviceList->HasSelection())
        return event.Skip();

    auto& services = m_exportBtn->GetValue() ? m_export : m_import;
    auto iter = services.find(m_serviceList->GetItemData(m_serviceList->GetSelection()));
    WebPier::ServicePtr service = iter != services.end() ? iter->second : WebPier::ServicePtr();

    try
    {
        if (!service)
            throw std::runtime_error(_("service data is not found"));

        CServiceDialog dialog(m_config, service, this);
        if (dialog.ShowModal() == wxID_OK && service->IsDirty())
        {
            service->Store();
            auto row = m_serviceList->GetSelectedRow();
            m_serviceList->DeleteItem(row);
            m_serviceList->InsertItem(row, ToVariantList(service), wxUIntPtr(service.get()));
            m_serviceList->SelectRow(row);
        }
    }
    catch (const std::exception& ex)
    {
        if (service)
            service->Revert();

        CMessageDialog dialog(this, _("Can't change service: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onImportMenuSelection(wxCommandEvent& event)
{
    try
    {
        wxFileDialog fileDialog(this, _("Open advertisement file"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Downloads), "", "*.json", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

        if (fileDialog.ShowModal() == wxID_CANCEL)
            return;

        WebPier::Exchange data;
        WebPier::ReadExchangeFile(fileDialog.GetPath(), data);

        if (m_config->Host == data.Pier)
        {
            CMessageDialog dialog(this, _("Peer name is the same as the local pier"), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
            dialog.ShowModal();
            return;
        }

        bool replacePeer = false;
        bool creategPeer = WebPier::IsUnknownPeer(data.Pier);
        if (!creategPeer)
        {
            if (data.Certificate != WebPier::GetCertificate(data.Pier))
            {
                CMessageDialog dialog(this, _("Such peer is already exists, but has a different certificate. Do you want to replace existing peer and its services?"), wxDEFAULT_DIALOG_STYLE | wxICON_QUESTION);
                if (dialog.ShowModal() != wxID_YES)
                    return;

                replacePeer = true;
            }
        }

        if (replacePeer)
        {
            for (auto& item : m_export)
                item.second->DelPeer(data.Pier);
        }

        CExchangeDialog dialog(data.Pier, data.Services, m_export, this);
        if (dialog.ShowModal() != wxID_OK)
            return;

        if (replacePeer)
            WebPier::DelPeer(data.Pier);

        if (creategPeer || replacePeer)
            WebPier::AddPeer(data.Pier, data.Certificate);

        auto exports = dialog.GetExport();
        auto imports = dialog.GetImport();

        for (auto& item : exports)
        {
            item.second->AddPeer(data.Pier);
            item.second->Store();
        }

        if (dialog.NeedImportMerge())
        {
            for (auto& item : imports)
            {
                WebPier::ServicePtr next = item.second;
                
                WebPier::ServicePtr curr;
                for (auto& pair : m_import)
                {
                    if (pair.second->Id == next->Id)
                    {
                        curr = pair.second;
                        break;
                    }
                }

                if (curr)
                {
                    if (!WebPier::IsEqual(curr, next))
                    {
                        CMessageDialog dialog(this, wxString::Format(_("Service '%s' is already exist, but differs from the new one. Do you want to replace it?"), next->Id), wxDEFAULT_DIALOG_STYLE | wxICON_QUESTION);
                        if (dialog.ShowModal() == wxID_YES)
                        {
                            curr->Purge();
                            next->Store();
                        }
                    }
                }
                else
                    next->Store();
            }
        }
        else
        {
            for (auto& item : m_import)
            {
                if (item.second->Peer == data.Pier)
                    item.second->Purge();
            }
            for (auto& item : imports)
                item.second->Store();
        }

        populate();

        if (dialog.NeedExportReply())
        {
            wxFileDialog fileDialog(this, _("Save advertisement file"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Desktop), "", "*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
            if (fileDialog.ShowModal() == wxID_CANCEL)
                return;

            WebPier::Exchange data { m_config->Host, WebPier::GetCertificate(m_config->Host), exports };
            WebPier::WriteExchangeFile(fileDialog.GetPath(), data);
        }
    }
    catch (const std::exception& ex)
    {
        CMessageDialog dialog(this, _("Can't introduce peer: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onExportMenuSelection(wxCommandEvent& event)
{
    try
    {
        CExchangeDialog dialog(m_config->Host, m_export, this);
        if (dialog.ShowModal() != wxID_OK)
            return;

        wxFileDialog fileDialog(this, _("Save advertisement file"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Desktop), "", "*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
        if (fileDialog.ShowModal() == wxID_CANCEL)
            return;

        WebPier::Exchange data { m_config->Host, WebPier::GetCertificate(m_config->Host), dialog.GetExport() };
        WebPier::WriteExchangeFile(fileDialog.GetPath(), data);
    }
    catch (const std::exception& ex)
    {
        for (auto& item : m_export)
            item.second->Revert();

        CMessageDialog dialog(this, _("Can't advertise host: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onDeleteServiceButtonClick(wxCommandEvent& event)
{
    if (!m_serviceList->HasSelection())
        return event.Skip();
    try
    {
        auto& services = m_exportBtn->GetValue() ? m_export : m_import;
        auto iter = services.find(m_serviceList->GetItemData(m_serviceList->GetSelection()));
        if (iter == services.end())
            throw std::runtime_error(_("service data is not found"));

        auto service = iter->second;
        CMessageDialog dialog(this, _("Do you want to remove service ") + service->Id, wxDEFAULT_DIALOG_STYLE | wxICON_QUESTION);
        if (dialog.ShowModal() == wxID_YES)
        {
            service->Purge();

            m_serviceList->DeleteItem(m_serviceList->GetSelectedRow());
            services.erase(iter);
        }
    }
    catch (const std::exception& ex)
    {
        CMessageDialog dialog(this, _("Can't remove service: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onAboutMenuSelection(wxCommandEvent& event)
{
    CAboutDialog dialog(this);
    dialog.ShowModal();
}

void CMainFrame::onImportRadioClick(wxCommandEvent& event)
{
    populate();
}

void CMainFrame::onExportRadioClick(wxCommandEvent& event)
{
    populate();
}
