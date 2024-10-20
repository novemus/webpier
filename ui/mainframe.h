#pragma once

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
    wxRadioButton* m_remoteBtn;
    wxRadioButton* m_localBtn;
    wxStaticText* m_hostLabel;
    wxBitmapButton* m_addBtn;
    wxBitmapButton* m_editBtn;
    wxBitmapButton* m_deleteBtn;
    wxDataViewListCtrl* m_serviceList;
    wxStatusBar* m_statusBar;

protected:

    void onClose(wxCloseEvent& event)
    {
        if (event.CanVeto())
        {
            event.Veto();
            this->Show(false);
        }
    }

    void onSettingsMenuSelection(wxCommandEvent& event);
    void onImportMenuSelection(wxCommandEvent& event);
    void onExportMenuSelection(wxCommandEvent& event);

    void onExitMenuSelection(wxCommandEvent& event)
    {
        this->Show(false);
    }

    void onAboutMenuSelection(wxCommandEvent& event);

    void onRemoteRadioClick(wxCommandEvent& event)
    {
        event.Skip();
    }

    void onLocalRadioClick(wxCommandEvent& event)
    {
        event.Skip();
    }

    void onAddServiceButtonClick(wxCommandEvent& event);
    void onEditServiceButtonClick(wxCommandEvent& event);

    void onDeleteServiceButtonClick(wxCommandEvent& event)
    {
        event.Skip();
    }

    void onServiceItemContextMenu(wxDataViewEvent& event)
    {
        event.Skip();
    }

    void populate();

public:

    CMainFrame();
    ~CMainFrame();

    bool Show(bool show = true) wxOVERRIDE;
};
