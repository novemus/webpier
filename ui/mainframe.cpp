#include "mainframe.h"
#include "context.h"
#include "servicedialog.h"
#include "settingsdialog.h"
#include "importdialog.h"
#include "exportdialog.h"
#include "aboutdialog.h"
#include "messagedialog.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

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

    m_remoteBtn = new wxRadioButton( this, wxID_ANY, _("Remote"), wxDefaultPosition, wxDefaultSize, 0 );
    m_remoteBtn->SetValue( true );
    topSizer->Add( m_remoteBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_localBtn = new wxRadioButton( this, wxID_ANY, _("Local"), wxDefaultPosition, wxDefaultSize, 0 );
    topSizer->Add( m_localBtn, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

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
    m_serviceList->AppendIconTextColumn( _("Id"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Peer"), wxDATAVIEW_CELL_INERT, 350, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Service"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Gateway"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Rendezvous"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    m_serviceList->AppendTextColumn( _("Autostart"), wxDATAVIEW_CELL_INERT, 100, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE|wxDATAVIEW_COL_SORTABLE )->GetRenderer()->EnableEllipsize( wxELLIPSIZE_END );
    mainSizer->Add( m_serviceList, 1, wxALL|wxEXPAND, 5 );

    this->SetSizer( mainSizer );
    this->Layout();
    m_statusBar = this->CreateStatusBar( 1, wxSTB_SIZEGRIP, wxID_ANY );

    this->Centre( wxBOTH );

    // Connect Events
    this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( CMainFrame::onClose ) );
    fileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onSettingsMenuSelection ), this, settingsItem->GetId());
    fileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onImportMenuSelection ), this, m_importItem->GetId());
    fileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onExportMenuSelection ), this, m_exportItem->GetId());
    fileMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onExitMenuSelection ), this, exitItem->GetId());
    helpMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CMainFrame::onAboutMenuSelection ), this, aboutItem->GetId());
    m_remoteBtn->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( CMainFrame::onRemoteRadioClick ), NULL, this );
    m_localBtn->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( CMainFrame::onLocalRadioClick ), NULL, this );
    m_addBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CMainFrame::onAddServiceButtonClick ), NULL, this );
    m_editBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CMainFrame::onEditServiceButtonClick ), NULL, this );
    m_deleteBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CMainFrame::onDeleteServiceButtonClick ), NULL, this );
    m_serviceList->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler( CMainFrame::onServiceItemContextMenu ), NULL, this );
}

CMainFrame::~CMainFrame()
{
    delete m_remoteBtn;
    delete m_localBtn;
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
        m_hostLabel->SetLabel(WebPier::GetConfig().GetHost());
        m_serviceList->DeleteAllItems();

        m_services.clear();
        m_services = m_remoteBtn->GetValue()
            ? WebPier::GetRemoteServices() 
            : WebPier::GetLocalServices();

        for (auto& item : m_services)
            m_serviceList->AppendItem(item.ToListView(), reinterpret_cast<wxUIntPtr>(&item));

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

void CMainFrame::onSettingsMenuSelection(wxCommandEvent& event)
{
    try
    {
        CSettingsDialog dialog(this);
        if (dialog.ShowModal() == wxID_OK)
        {
            if (WebPier::GetHost() != m_hostLabel->GetLabel())
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
    WebPier::Service service(m_localBtn->GetValue());

    CServiceDialog dialog(&service, this);
    if (dialog.ShowModal() == wxID_OK)
    {
        try
        {
            service.Store();
            m_services.push_back(service);
            m_serviceList->AppendItem(service.ToListView(), reinterpret_cast<wxUIntPtr>(&m_services.back()));
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

    WebPier::Service* service = reinterpret_cast<WebPier::Service*>(m_serviceList->GetItemData(m_serviceList->GetSelection()));

    CServiceDialog dialog(service, this);
    if (dialog.ShowModal() == wxID_OK)
    {
        try
        {
            service->Store();
            auto row = m_serviceList->GetSelectedRow();
            m_serviceList->DeleteItem(row);
            m_serviceList->InsertItem(row, service->ToListView(), reinterpret_cast<wxUIntPtr>(service));
            m_serviceList->SelectRow(row);
        }
        catch (const std::exception& ex)
        {
            CMessageDialog dialog(this, _("Can't change service: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
            dialog.ShowModal();
        }
    }
}

void CMainFrame::onImportMenuSelection(wxCommandEvent& event)
{
    try
    {
        wxFileDialog fileDialog(this, _("Open advertisement file"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Downloads), "", "*.json", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

        if (fileDialog.ShowModal() == wxID_CANCEL)
            return;

        WebPier::Exchange data {};
        WebPier::ReadExchangeFile(fileDialog.GetPath(), data);

        CImportDialog dialog(this);
        if (dialog.ShowModal() != wxID_OK)
            return;
    }
    catch (const std::exception &ex)
    {
        CMessageDialog dialog(this, _("Can't intergate peer: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onExportMenuSelection(wxCommandEvent& event)
{
    try
    {
        CExportDialog dialog(this);
        if (dialog.ShowModal() != wxID_OK)
            return;

        wxFileDialog fileDialog(this, _("Save advertisement file"), wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir_Desktop), "", "*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
        if (fileDialog.ShowModal() == wxID_CANCEL)
            return;

        WebPier::Exchange data { WebPier::GetHost(), WebPier::GetCertificate(WebPier::GetHost()), dialog.GetExport() };
        WebPier::WriteExchangeFile(fileDialog.GetPath(), data);
    }
    catch (const std::exception &ex)
    {
        CMessageDialog dialog(this, _("Can't advertise host: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
}

void CMainFrame::onDeleteServiceButtonClick(wxCommandEvent& event)
{
    if (!m_serviceList->HasSelection())
        return event.Skip();

    WebPier::Service* service = reinterpret_cast<WebPier::Service*>(m_serviceList->GetItemData(m_serviceList->GetSelection()));

    CMessageDialog dialog(nullptr, _("Do you want to remove service ") + service->GetId(), wxDEFAULT_DIALOG_STYLE|wxICON_QUESTION);
    if (dialog.ShowModal() == wxID_YES)
    {
        try
        {
            service->Purge();

            m_serviceList->DeleteItem(m_serviceList->GetSelectedRow());
            m_services.erase(std::remove_if(m_services.begin(), m_services.end(), [service](const auto& item) {
                return &item == service;
            }), m_services.end());
        }
        catch (const std::exception& ex)
        {
            CMessageDialog dialog(this, _("Can't remove service: ") + ex.what(), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
            dialog.ShowModal();
        }
    }
}

void CMainFrame::onAboutMenuSelection(wxCommandEvent& event)
{
    CAboutDialog dialog(this);
    dialog.ShowModal();
}

void CMainFrame::onRemoteRadioClick(wxCommandEvent& event)
{
    populate();
}

void CMainFrame::onLocalRadioClick(wxCommandEvent& event)
{
    populate();
}
