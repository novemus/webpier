#include "mainframe.h"
#include "context.h"
#include "servicedialog.h"
#include "settingsdialog.h"
#include "exchangedialog.h"
#include "aboutdialog.h"
#include "messagedialog.h"
#include "logo.h"
#include <wx/stdpaths.h>

wxVector<wxVariant> ToVariantList(const WebPier::Daemon::Handle& handle, WebPier::Context::ServicePtr service)
{
    WebPier::Daemon::Health health = WebPier::Daemon::Status(handle);

    const wxBitmap& bitmap = health.State == WebPier::Daemon::Health::ASLEEP 
        ? ::GetGreyCircleImage() : health.State == WebPier::Daemon::Health::ACTIVE 
        ? ::GetGreenCircleImage() : ::GetRedCircleImage();

    wxVector<wxVariant> data;
    data.push_back(wxVariant(wxDataViewIconText(service->Name, bitmap)));
    data.push_back(wxVariant(wxString(service->Pier)));
    data.push_back(wxVariant(wxString(service->Address)));
    data.push_back(wxVariant(wxString(service->Rendezvous.IsEmpty() ? wxT("Email") : wxT("DHT"))));
    data.push_back(wxVariant(wxString(service->Autostart ? _("yes") : _("no"))));
    return data;
}

CMainFrame::CMainFrame(const wxIcon& icon) : wxFrame(nullptr, wxID_ANY, wxT("WebPier"), wxDefaultPosition, wxSize(950, 500), wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
    this->SetIcon(icon);
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
    delete m_pierLabel;
    delete m_addBtn;
    delete m_editBtn;
    delete m_deleteBtn;
    delete m_serviceList;
    delete m_statusBar;
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
        {
            WebPier::Daemon::Handle handle{m_importBtn->GetValue() ? item.second->Pier : m_config->Pier, item.second->Name};
            m_serviceList->AppendItem(ToVariantList(handle, item.second), item.first);
        }

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
            if (m_config->Pier != m_pierLabel->GetLabel())
                Populate();
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

            WebPier::Daemon::Handle handle{m_importBtn->GetValue() ? service->Pier : m_config->Pier, service->Name};
            m_serviceList->AppendItem(ToVariantList(handle, service), wxUIntPtr(service.get()));
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
    WebPier::Context::ServicePtr service = iter != services.end() ? iter->second : WebPier::Context::ServicePtr();

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

            WebPier::Daemon::Handle handle{m_importBtn->GetValue() ? service->Pier : m_config->Pier, service->Name};
            m_serviceList->InsertItem(row, ToVariantList(handle, service), wxUIntPtr(service.get()));
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
        wxFileDialog fileDialog(this, _("Open offer"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Downloads), "", "*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

        if (fileDialog.ShowModal() == wxID_CANCEL)
            return;

        WebPier::Context::Offer offer;
        WebPier::Context::ReadOffer(fileDialog.GetPath(), offer);

        if (m_config->Pier == offer.Pier)
        {
            CMessageDialog dialog(this, _("Pier name is the same as the local pier"), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
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
                        CMessageDialog dialog(this, wxString::Format(_("Service '%s' is already imported from '%s', but differs from the new one. Do you want to replace it?"), next->Name, next->Pier), wxDEFAULT_DIALOG_STYLE | wxICON_QUESTION);
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
                if (item.second->Pier == offer.Pier)
                    item.second->Purge();
            }
            for (auto& item : imports)
                item.second->Store();
        }

        Populate();

        if (dialog.NeedExportReply())
        {
            wxFileDialog fileDialog(this, _("Save offer"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Desktop), "", "*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
            if (fileDialog.ShowModal() == wxID_CANCEL)
                return;

            WebPier::Context::WriteOffer(fileDialog.GetPath(), WebPier::Context::Offer{ m_config->Pier, WebPier::Context::GetCertificate(m_config->Pier), exports });
        }
    }
    catch (const std::exception& ex)
    {
        CMessageDialog dialog(this, _("Can't introduce pier: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
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
        CMessageDialog dialog(this, _("Do you want to remove service ") + service->Name, wxDEFAULT_DIALOG_STYLE | wxICON_QUESTION);
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
    Populate();
}

void CMainFrame::onExportRadioClick(wxCommandEvent& event)
{
    Populate();
}
