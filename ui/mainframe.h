#pragma once

#include "context.h"
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dataview.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include <wx/aboutdlg.h> 

class CMainFrame : public wxFrame
{
    wxMenuItem* m_importItem;
    wxMenuItem* m_exportItem;
    wxRadioButton* m_importBtn;
    wxRadioButton* m_exportBtn;
    wxStaticText* m_hostLabel;
    wxBitmapButton* m_addBtn;
    wxBitmapButton* m_editBtn;
    wxBitmapButton* m_deleteBtn;
    wxDataViewListCtrl* m_serviceList;
    wxStatusBar* m_statusBar;
    WebPier::ConfigPtr m_config;
    WebPier::ServiceList m_export;
    WebPier::ServiceList m_import;

protected:

    void onSettingsMenuSelection(wxCommandEvent& event);
    void onImportMenuSelection(wxCommandEvent& event);
    void onExportMenuSelection(wxCommandEvent& event);
    void onExitMenuSelection(wxCommandEvent& event);
    void onAboutMenuSelection(wxCommandEvent& event);
    void onImportRadioClick(wxCommandEvent& event);
    void onExportRadioClick(wxCommandEvent& event);
    void onAddServiceButtonClick(wxCommandEvent& event);
    void onEditServiceButtonClick(wxCommandEvent& event);
    void onDeleteServiceButtonClick(wxCommandEvent& event);
    void onServiceItemContextMenu(wxDataViewEvent& event) { event.Skip(); }

public:

    CMainFrame();
    ~CMainFrame();

    void Populate();
    wxMenu* BuildImportMenu();
    wxMenu* BuildExportMenu();
};
