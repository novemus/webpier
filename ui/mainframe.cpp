#include "mainframe.h"
#include "servicedialog.h"
#include "settingsdialog.h"
#include "importdialog.h"
#include "exportdialog.h"
#include "aboutdialog.h"

CMainFrame::CMainFrame() : wxFrame(nullptr, wxID_ANY, _("WebPier"), wxDefaultPosition, wxSize(800, 500), wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_MENU ) );

    wxMenuBar* menubar;
    menubar = new wxMenuBar( 0 );
    wxMenu* fileMenu;
    fileMenu = new wxMenu();
    wxMenuItem* settingsItem;
    settingsItem = new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Settings...") ) , wxEmptyString, wxITEM_NORMAL );
    fileMenu->Append( settingsItem );

    fileMenu->AppendSeparator();

    wxMenuItem* m_importItem;
    m_importItem = new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Import...") ) , wxEmptyString, wxITEM_NORMAL );
    fileMenu->Append( m_importItem );

    wxMenuItem* m_exportItem;
    m_exportItem = new wxMenuItem( fileMenu, wxID_ANY, wxString( _("&Export...") ) , wxEmptyString, wxITEM_NORMAL );
    fileMenu->Append( m_exportItem );

    fileMenu->AppendSeparator();

    wxMenuItem* exitItem;
    exitItem = new wxMenuItem( fileMenu, wxID_ANY, wxString( _("E&xit") ) , wxEmptyString, wxITEM_NORMAL );
    fileMenu->Append( exitItem );

    menubar->Append( fileMenu, _("&File") );

    wxMenu* helpMenu;
    helpMenu = new wxMenu();
    wxMenuItem* aboutItem;
    aboutItem = new wxMenuItem( helpMenu, wxID_ANY, wxString( _("&About...") ) , wxEmptyString, wxITEM_NORMAL );
    helpMenu->Append( aboutItem );

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

	m_hostLabel = new wxStaticText( this, wxID_ANY, _("indefinite"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL );
	m_hostLabel->Wrap( -1 );
	topSizer->Add( m_hostLabel, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    m_addBtn = new wxBitmapButton( this, wxID_ANY, wxArtProvider::GetBitmapBundle(wxART_NEW, wxART_BUTTON, wxSize(16, 16)), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
    topSizer->Add( m_addBtn, 0, wxALL, 5 );

    m_editBtn = new wxBitmapButton( this, wxID_ANY, wxArtProvider::GetBitmapBundle(wxART_EDIT, wxART_BUTTON, wxSize(16, 16)), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
    topSizer->Add( m_editBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    m_deleteBtn = new wxBitmapButton( this, wxID_ANY, wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_BUTTON, wxSize(16, 16)), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW|0 );
    topSizer->Add( m_deleteBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );


    mainSizer->Add( topSizer, 0, wxEXPAND, 5 );

    m_serviceList = new wxDataViewListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES|wxDV_ROW_LINES|wxDV_SINGLE );
    m_serviceList->AppendIconTextColumn( _("Service"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
    m_serviceList->AppendTextColumn( _("Peer"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
    m_serviceList->AppendTextColumn( _("Mapping"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
    m_serviceList->AppendTextColumn( _("Gateway"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
    m_serviceList->AppendTextColumn( _("Rendezvous"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
    m_serviceList->AppendTextColumn( _("Autostart"), wxDATAVIEW_CELL_INERT, 150, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
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

void CMainFrame::onSettingsMenuSelection(wxCommandEvent& event)
{
    CSettingsDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
        std::cout << "cancel\n";
    else
        std::cout << "ok\n";
}

void CMainFrame::onAddServiceButtonClick(wxCommandEvent& event)
{
    CServiceDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
        std::cout << "cancel\n";
    else
        std::cout << "ok\n";
}

void CMainFrame::onEditServiceButtonClick(wxCommandEvent& event)
{
    CServiceDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
        std::cout << "cancel\n";
    else
        std::cout << "ok\n";
}

void CMainFrame::onImportMenuSelection(wxCommandEvent& event)
{
    wxFileDialog fileDialog(this, _("Open exchange file"), "", "", "*.*", wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (fileDialog.ShowModal() == wxID_CANCEL)
        return;

    CImportDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
        std::cout << "cancel\n";
    else
        std::cout << "ok\n";
}

void CMainFrame::onExportMenuSelection(wxCommandEvent& event)
{
    CExportDialog dialog(this);
    if (dialog.ShowModal() != wxID_OK)
        std::cout << "cancel\n";
    else
    {
        wxFileDialog fileDialog(this, _("Save exchange file"), "", "", "*.*", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
        if (fileDialog.ShowModal() == wxID_CANCEL)
            return;

        std::cout << "ok\n";
    }
}

void CMainFrame::onAboutMenuSelection(wxCommandEvent& event)
{
    CAboutDialog dialog(this);
    dialog.ShowModal();
}
