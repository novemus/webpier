#include <ui/mainframe.h>
#include <ui/context.h>
#include <ui/servicedialog.h>
#include <ui/settingsdialog.h>
#include <ui/exchangedialog.h>
#include <ui/aboutdialog.h>
#include <ui/messagedialog.h>
#include <ui/logo.h>
#include <wx/stdpaths.h>
#include <wx/notifmsg.h>

const wxBitmap& GetStatusBitmap(WebPier::Daemon::Health::Status state)
{
    switch(state)
    {
        case WebPier::Daemon::Health::Asleep:
            return ::GetGreyCircleImage();
        case WebPier::Daemon::Health::Broken:
            return ::GetRedCircleImage();
        case WebPier::Daemon::Health::Lonely:
            return ::GetBlueCircleImage();
        default:
            return ::GetGreenCircleImage();
    }

    return ::GetGreyCircleImage();
}

wxString Stringify(WebPier::Daemon::Health::Status state)
{
    switch(state)
    {
        case WebPier::Daemon::Health::Asleep:
            return _("asleep");
        case WebPier::Daemon::Health::Broken:
            return _("broken");
        case WebPier::Daemon::Health::Lonely:
            return _("lonely");
        default:
            return _("burden");
    }

    return _("unknown");
}

wxVector<wxVariant> CMainFrame::makeListItem(WebPier::Context::ServicePtr service)
{
    WebPier::Daemon::Health::Status state = service->Autostart ? WebPier::Daemon::Health::Lonely : WebPier::Daemon::Health::Asleep;

    auto owner = service->IsExport() ? WebPier::Context::Pier() : service->Pier;
    for (const auto& item : m_status)
    {
        if (item.Pier == owner && item.Service == service->Name)
            state = item.State;
    }

    wxVector<wxVariant> data;
    data.push_back(wxVariant(wxDataViewIconText(service->Name, GetStatusBitmap(state))));
    data.push_back(wxVariant(wxString(service->Pier)));
    data.push_back(wxVariant(wxString(service->Address)));
    data.push_back(wxVariant(wxString(service->Rendezvous.IsEmpty() ? wxT("Email") : wxT("DHT"))));
    data.push_back(wxVariant(wxString(service->Autostart ? _("yes") : _("no"))));
    return data;
}

CMainFrame::CMainFrame() : wxFrame(nullptr, wxID_ANY, wxT("WebPier"), wxDefaultPosition, wxSize(950, 500), wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
    this->SetIcon(::GetAppIconBundle().GetIcon());
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );

    wxMenuBar* menubar = new wxMenuBar( 0 );
    wxMenu* fileMenu = new wxMenu();
    wxMenuItem* settingsItem = fileMenu->Append( new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Settings...") ) , wxEmptyString, wxITEM_NORMAL ) );

    fileMenu->AppendSeparator();

    m_exportItem = fileMenu->Append( new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Create an offer...") ) , wxEmptyString, wxITEM_NORMAL ) );
    m_exportItem->Enable(false);

    m_importItem = fileMenu->Append( new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Upload an offer...") ) , wxEmptyString, wxITEM_NORMAL ) );
    m_importItem->Enable(false);

    fileMenu->AppendSeparator();
    wxMenuItem* exitItem = fileMenu->Append( new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Exit") ) , wxEmptyString, wxITEM_NORMAL ) );

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

	m_pierLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL );
	m_pierLabel->Wrap( -1 );
	topSizer->Add( m_pierLabel, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

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
    m_serviceList->AppendTextColumn( _("Pier"), wxDATAVIEW_CELL_INERT, 350, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Address"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Rendezvous"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Autostart"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    mainSizer->Add( m_serviceList, 1, wxALL|wxEXPAND, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    m_statusBar = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );
    m_statusBar->SetFieldsCount(2);

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
    m_serviceList->Connect( wxEVT_COMMAND_DATAVIEW_SELECTION_CHANGED, wxDataViewEventHandler( CMainFrame::onServiceItemSelectionChanged ), NULL, this );

    m_timer = new wxTimer(this);
    this->Bind( wxEVT_TIMER, wxTimerEventHandler(CMainFrame::onStatusTimeout), this, m_timer->GetId());
    m_timer->Start(500, true);
}

CMainFrame::~CMainFrame()
{
    delete m_importBtn;
    delete m_exportBtn;
    delete m_pierLabel;
    delete m_addBtn;
    delete m_editBtn;
    delete m_deleteBtn;
    delete m_serviceList;
    delete m_statusBar;
    delete m_timer;
}

void CMainFrame::RefreshStatus(const WebPier::Daemon::Handle& handle)
{
    WebPier::Daemon::Health health { handle, WebPier::Daemon::Health::Broken };
    try
    {
        health = WebPier::Daemon::Status(handle);
    }
    catch(const std::exception& ex)
    {
        wxNotificationMessage msg(wxEmptyString, _("Can't get the pier status. ") + ex.what(), nullptr, wxICON_ERROR);
        msg.Show();
    }

    for (auto& status : m_status)
    {
        if (handle.Pier == status.Pier && handle.Service == status.Service)
        {
            status = health;
            break;
        }
    }

    int current = m_serviceList->GetSelectedRow();
    for(int i = 0; i < m_serviceList->GetItemCount(); ++i)
    {
        wxVariant value;
        m_serviceList->GetValue(value, i, 0);

        wxString service = value.GetAny().As<wxDataViewIconText>().GetText();
        wxString pier = m_exportBtn->GetValue() ? WebPier::Context::Pier() : m_serviceList->GetTextValue(i, 1);

        if (handle.Service == service && handle.Pier == pier)
        {
            m_serviceList->SetValue(wxVariant(wxDataViewIconText(service, GetStatusBitmap(health.State))), i, 0);
            if (current == i)
            {
                m_statusBar->SetStatusText(Stringify(health.State), 0);
                m_statusBar->SetStatusText(health.Message, 1);
            }
            break;
        }
    }
}

void CMainFrame::RefreshStatus()
{
    try
    {
        m_status = WebPier::Daemon::Status();
    }
    catch(const std::exception& ex)
    {
        wxNotificationMessage msg(wxEmptyString, _("Can't refresh the status. ") + ex.what(), nullptr, wxICON_ERROR);
        msg.Show();
        m_status.clear();
    }

    int current = m_serviceList->GetSelectedRow();
    for(int i = 0; i < m_serviceList->GetItemCount(); ++i)
    {
        wxVariant value;
        m_serviceList->GetValue(value, i, 0);

        wxString service = value.GetAny().As<wxDataViewIconText>().GetText();
        wxString pier = m_exportBtn->GetValue() ? WebPier::Context::Pier() : m_serviceList->GetTextValue(i, 1);

        WebPier::Daemon::Health health = { {pier, service }, WebPier::Daemon::Health::Broken };
        for (const auto& item : m_status)
        {
            if (item.Pier == pier && item.Service == service)
            {
                health = item;
                break;
            }
        }

        m_serviceList->SetValue(wxVariant(wxDataViewIconText(service, GetStatusBitmap(health.State))), i, 0);
        if (current == i)
        {
            m_statusBar->SetStatusText(Stringify(health.State), 0);
            m_statusBar->SetStatusText(health.Message, 1);
        }
    }
}

void CMainFrame::onServiceItemSelectionChanged(wxDataViewEvent& event)
{
    int row = m_serviceList->ItemToRow(event.GetItem());
    if (row == wxNOT_FOUND)
    {
        m_statusBar->SetStatusText(wxEmptyString);
        return;
    }
    
    wxVariant value;
    m_serviceList->GetValue(value, row, 0);

    wxString service = value.GetAny().As<wxDataViewIconText>().GetText();
    wxString pier = m_exportBtn->GetValue() ? WebPier::Context::Pier() : m_serviceList->GetTextValue(row, 1);

    for (const auto& item : m_status)
    {
        if (item.Pier == pier && item.Service == service)
        {
            m_statusBar->SetStatusText(Stringify(item.State));
            break;
        }
    }
}

void CMainFrame::onServiceItemContextMenu(wxDataViewEvent& event)
{
    int row = m_serviceList->ItemToRow(event.GetItem());
    if (row == wxNOT_FOUND)
        return;
    
    wxVariant value;
    m_serviceList->GetValue(value, row, 0);

    wxString service = value.GetAny().As<wxDataViewIconText>().GetText();
    wxString pier = m_exportBtn->GetValue() ? WebPier::Context::Pier() : m_serviceList->GetTextValue(row, 1);

    try
    {
        auto info = WebPier::Daemon::Review(WebPier::Daemon::Handle{pier, service});

        wxMenu* menu = new wxMenu();
        if (info.State == WebPier::Daemon::Health::Asleep)
        {
            menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this, info](wxCommandEvent&)
            {
                try
                {
                    WebPier::Daemon::Engage(info);
                }
                catch(const std::exception& ex)
                {
                    wxNotificationMessage msg(wxEmptyString, _("Can't start the pier. ") + ex.what(), nullptr, wxICON_ERROR);
                    msg.Show();
                }
                RefreshStatus(info);
            }, 
            menu->Append(wxID_ANY, _("&Engage"))->GetId());
        }
        else
        {
            menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this, info](wxCommandEvent&)
            {
                try
                {
                    WebPier::Daemon::Unplug(info);
                }
                catch(const std::exception& ex)
                {
                    wxNotificationMessage msg(wxEmptyString, _("Can't stop the pier. ") + ex.what(), nullptr, wxICON_ERROR);
                    msg.Show();
                }
                RefreshStatus(info);
            }, menu->Append(wxID_ANY, _("&Unplug"))->GetId());

            if (info.State == WebPier::Daemon::Health::Burden || !info.Tunnels.empty())
            {
                wxMenu* tunnels = new wxMenu();
                for (auto& tunnel : info.Tunnels)
                {
                    tunnels->Append(wxID_ANY, wxString::Format(wxT("pier=%s pid=%d"), tunnel.Pier, tunnel.Pid))->Enable(false);
                }
                menu->Append(wxID_ANY, "&Tunnels", tunnels);
            }
        }

        PopupMenu(menu);
    }
    catch(const std::exception& ex)
    {
        wxNotificationMessage msg(wxEmptyString, _("Can't popup the menu. ") + ex.what(), nullptr, wxICON_ERROR);
        msg.Show();
    }
}

void CMainFrame::onStatusTimeout(wxTimerEvent& event)
{
    if (m_config)
    {
        RefreshStatus();
        m_timer->Start(30000, true);
    }
    else 
    {
        m_timer->Start(5000, true);
    }
}

void CMainFrame::Populate()
{
    try
    {
        m_serviceList->DeleteAllItems();

        m_config = WebPier::Context::GetConfig();
        m_export = WebPier::Context::GetExportServices();
        m_import = WebPier::Context::GetImportServices();

        m_pierLabel->SetLabel(m_config->Pier);

        for (auto& item : m_importBtn->GetValue() ? m_import : m_export)
            m_serviceList->AppendItem(makeListItem(item.second), item.first);

        m_importItem->Enable(true);
        m_exportItem->Enable(true);
        m_addBtn->Enable(true);
        m_editBtn->Enable(true);
        m_deleteBtn->Enable(true);
    }
    catch(const std::exception& ex)
    {
        CMessageDialog dialog(this, _("Can't populate the service list. ") + ex.what(), wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
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
            if (m_config->Pier != m_pierLabel->GetLabel())
                Populate();
        }
    }
    catch (const std::exception &ex)
    {
        CMessageDialog dialog(this, _("Can't setup the settings. ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onAddServiceButtonClick(wxCommandEvent& event)
{
    WebPier::Context::ServicePtr service = m_exportBtn->GetValue() ? WebPier::Context::CreateExportService() : WebPier::Context::CreateImportService();

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

            m_serviceList->AppendItem(makeListItem(service), wxUIntPtr(service.get()));
        }
        catch(const std::exception& ex)
        {
            CMessageDialog dialog(this, _("Can't add the service. ") + ex.what(), wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
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
    WebPier::Context::ServicePtr service = iter != services.end() ? iter->second : WebPier::Context::ServicePtr();

    try
    {
        if (!service)
            throw std::runtime_error(_("The service item is not found"));

        CServiceDialog dialog(m_config, service, this);
        if (dialog.ShowModal() == wxID_OK && service->IsDirty())
        {
            service->Store();
            auto row = m_serviceList->GetSelectedRow();
            m_serviceList->DeleteItem(row);

            m_serviceList->InsertItem(row, makeListItem(service), wxUIntPtr(service.get()));
            m_serviceList->SelectRow(row);
        }
    }
    catch (const std::exception& ex)
    {
        if (service)
            service->Revert();

        CMessageDialog dialog(this, _("Can't change the service. ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onImportMenuSelection(wxCommandEvent& event)
{
    try
    {
        wxFileDialog fileDialog(this, _("Open offer"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Downloads), "", "*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

        if (fileDialog.ShowModal() == wxID_CANCEL)
            return;

        WebPier::Context::Offer offer;
        WebPier::Context::ReadOffer(fileDialog.GetPath(), offer);

        if (m_config->Pier == offer.Pier)
        {
            CMessageDialog dialog(this, _("The pier name is the same as the local pier"), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
            dialog.ShowModal();
            return;
        }

        bool replacePier = false;
        bool createPier = WebPier::Context::IsUnknownPier(offer.Pier);
        if (!createPier)
        {
            if (offer.Certificate != WebPier::Context::GetCertificate(offer.Pier))
            {
                CMessageDialog dialog(this, _("Such pier is already exists, but has a different certificate. Do you want to replace existing pier and its services?"), wxDEFAULT_DIALOG_STYLE | wxICON_QUESTION);
                if (dialog.ShowModal() != wxID_YES)
                    return;

                replacePier = true;
            }
        }

        if (replacePier)
        {
            for (auto& item : m_export)
                item.second->DelPier(offer.Pier);
        }

        CExchangeDialog dialog(offer.Pier, offer.Services, m_export, this);
        if (dialog.ShowModal() != wxID_OK)
            return;

        if (replacePier)
            WebPier::Context::DelPier(offer.Pier);

        if (createPier || replacePier)
            WebPier::Context::AddPier(offer.Pier, offer.Certificate);

        auto exports = dialog.GetExport();
        auto imports = dialog.GetImport();

        for (auto& item : m_export)
        {
            if (exports.find(item.first) == exports.end())
                item.second->DelPier(offer.Pier);
            else
                item.second->AddPier(offer.Pier);

            item.second->Store();
        }

        if (dialog.NeedImportMerge())
        {
            for (auto& item : imports)
            {
                WebPier::Context::ServicePtr next = item.second;
                WebPier::Context::ServicePtr curr;
                for (auto& pair : m_import)
                {
                    if (pair.second->Name == next->Name && pair.second->Pier == next->Pier)
                    {
                        curr = pair.second;
                        break;
                    }
                }

                if (curr)
                {
                    if (!WebPier::Context::IsEqual(curr, next))
                    {
                        CMessageDialog dialog(this, wxString::Format(_("The service '%s' is already imported from '%s', but differs from the new one. Do you want to replace it?"), next->Name, next->Pier), wxDEFAULT_DIALOG_STYLE | wxICON_QUESTION);
                        if (dialog.ShowModal() == wxID_YES)
                            next->Store();
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
                if (item.second->Pier == offer.Pier)
                    item.second->Purge();
            }
            for (auto& item : imports)
                item.second->Store();
        }

        Populate();

        if (dialog.NeedExportReply())
        {
            wxFileDialog fileDialog(this, _("Save the offer"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Desktop), "", "*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
            if (fileDialog.ShowModal() == wxID_CANCEL)
                return;

            WebPier::Context::WriteOffer(fileDialog.GetPath(), WebPier::Context::Offer{ m_config->Pier, WebPier::Context::GetCertificate(m_config->Pier), exports });
        }
    }
    catch (const std::exception& ex)
    {
        CMessageDialog dialog(this, _("Offer uploading failed. ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onExportMenuSelection(wxCommandEvent& event)
{
    try
    {
        CExchangeDialog dialog(m_config->Pier, m_export, this);
        if (dialog.ShowModal() != wxID_OK)
            return;

        wxFileDialog fileDialog(this, _("Save offer"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Desktop), "webpier.offer", "*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
        if (fileDialog.ShowModal() == wxID_CANCEL)
            return;

        WebPier::Context::WriteOffer(fileDialog.GetPath(), WebPier::Context::Offer { m_config->Pier, WebPier::Context::GetCertificate(m_config->Pier), dialog.GetExport() });
    }
    catch (const std::exception& ex)
    {
        for (auto& item : m_export)
            item.second->Revert();

        CMessageDialog dialog(this, _("Couldn't create the offer. ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
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
            throw std::runtime_error(_("The service item is not found"));

        auto service = iter->second;
        CMessageDialog dialog(this, _("Do you want to remove the service ") + service->Name, wxDEFAULT_DIALOG_STYLE | wxICON_QUESTION);
        if (dialog.ShowModal() == wxID_YES)
        {
            service->Purge();

            m_serviceList->DeleteItem(m_serviceList->GetSelectedRow());
            services.erase(iter);
        }
    }
    catch (const std::exception& ex)
    {
        CMessageDialog dialog(this, _("Can't remove the service. ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
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
    Populate();
}

void CMainFrame::onExportRadioClick(wxCommandEvent& event)
{
    Populate();
}
