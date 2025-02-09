#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/notifmsg.h>
#include <wx/snglinst.h>
#include <wx/stdpaths.h>
#include <ui/mainframe.h>
#include <assets/logo.h>
#include <assets/blue.h>
#include <assets/green.h>
#include <assets/red.h>
#include <assets/grey.h>

const wxBitmap& GetLogo()
{
    static const wxBitmap s_logo(wxBITMAP_PNG_FROM_DATA(webpier_logo));
    return s_logo;
}

const wxBitmap& GetBlueCircleImage()
{
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(blue_circle));
    return s_image;
}

const wxBitmap& GetGreenCircleImage()
{
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(green_circle));
    return s_image;
}

const wxBitmap& GetRedCircleImage()
{
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(red_circle));
    return s_image;
}

const wxBitmap& GetGreyCircleImage()
{
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(grey_circle));
    return s_image;
}

CMainFrame* CreateMainFrame(const wxIcon& icon)
{
    CMainFrame* frame = new CMainFrame(icon);

    frame->Populate();
    frame->Show(true);

    return frame;
}

class CTaskBarIcon : public wxTaskBarIcon
{
    CMainFrame* m_frame;

public:

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    CTaskBarIcon(const wxIcon& icon, wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE)
        : wxTaskBarIcon(iconType)
#else
    CTaskBarIcon(const wxIcon& icon)
#endif
    {
        this->SetIcon(icon);

        m_frame = CreateMainFrame(icon);
        m_frame->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( CTaskBarIcon::OnFrameClose ), NULL, this);
    }

    ~CTaskBarIcon() override
    {
        delete m_frame;
    }

protected:

    void OnLeftButtonDClick(wxTaskBarIconEvent&)
    {
        m_frame->Show(true);
    }

    void OnMenuConfigure(wxCommandEvent&)
    {
        m_frame->Show(true);
    }

    void OnMenuExit(wxCommandEvent&)
    {
        m_frame->Close(true);
        this->Destroy();
    }

    void OnFrameClose(wxCloseEvent& event)
    {
        if (wxTaskBarIcon::IsAvailable())
        {
            if (event.CanVeto())
            {
                event.Veto();
                m_frame->Show(false);
            }
        }
        else
        {
            this->Destroy();
        }
    }

    wxMenu* CreatePopupMenu() wxOVERRIDE
    {
        wxMenu* menu = new wxMenu();
        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CTaskBarIcon::OnMenuConfigure ), this, menu->Append(wxID_ANY, _("&Configure..."))->GetId());

        menu->AppendSeparator();

        wxMenu* imports = new wxMenu();
        wxMenu* exports = new wxMenu();
        
        bool isPassive = true;

        try
        {
            auto pier = WebPier::Context::Pier();
            auto status = WebPier::Daemon::Status();

            for (const auto& item : status)
            {
                wxMenu* menu = item.Pier == pier ? exports : imports;
                wxMenuItem* check = menu->AppendCheckItem(wxID_ANY, item.Pier + wxT(":") + item.Service);

                if (item.State == WebPier::Daemon::Health::Asleep)
                {
                    check->Check(false);
                    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this, item](wxCommandEvent&)
                    {
                        try
                        {
                            WebPier::Daemon::Engage(item);
                        }
                        catch(const std::exception& ex)
                        {
                            wxNotificationMessage msg(wxEmptyString, _("Can't start the pier: ") + ex.what(), nullptr, wxICON_ERROR);
                            msg.Show();
                        }
                        m_frame->RefreshStatus(item);
                    }, check->GetId());
                }
                else
                {
                    isPassive = false;
                    check->Check(true);
                    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this, item](wxCommandEvent&)
                    {
                        try
                        {
                            WebPier::Daemon::Unplug(item);
                        }
                        catch(const std::exception& ex)
                        {
                            wxNotificationMessage msg(wxEmptyString, _("Can't stop the pier: ") + ex.what(), nullptr, wxICON_ERROR);
                            msg.Show();
                        }
                        m_frame->RefreshStatus(item);
                    }, check->GetId());
                }
            }
        }
        catch(const std::exception& ex)
        {
            wxNotificationMessage msg(wxEmptyString, _("Can't popup menu: ") + ex.what(), nullptr, wxICON_ERROR);
            msg.Show();
        }

        menu->Append(wxID_ANY, "&Import", imports);
        menu->Append(wxID_ANY, "&Export", exports);

        wxMenuItem* unplug = menu->Append(wxID_ANY, _("&Unplug"));
        wxMenuItem* reboot = menu->Append(wxID_ANY, _("&Reboot"));

        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&)
        {
            try
            {
                WebPier::Daemon::Unplug();
            }
            catch(const std::exception& ex)
            {
                wxNotificationMessage msg(wxEmptyString, _("Can't unplug the pier: ") + ex.what(), nullptr, wxICON_ERROR);
                msg.Show();
            }
            m_frame->RefreshStatus();
        }, unplug->GetId());

        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&)
        {
            try
            {
                WebPier::Daemon::Engage();
            }
            catch(const std::exception& ex)
            {
                wxNotificationMessage msg(wxEmptyString, _("Can't reboot the pier: ") + ex.what(), nullptr, wxICON_ERROR);
                msg.Show();
            }
            m_frame->RefreshStatus();
        }, reboot->GetId());

        if (isPassive)
            unplug->Enable(false);

        menu->AppendSeparator();

#ifdef __WXOSX__
        if (OSXIsStatusItem())
#endif
        {
            wxMenuItem* exit = menu->Append(wxID_ANY, _("&Quit"));
            menu->Bind(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( CTaskBarIcon::OnMenuExit ), this, exit->GetId());
        }

        return menu;
    }

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(CTaskBarIcon, wxTaskBarIcon)
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLeftButtonDClick)
wxEND_EVENT_TABLE()

class CWebPierApp : public wxApp
{
    wxSingleInstanceChecker* m_checker;

public:

    bool OnInit() override
    {
        if (!wxApp::OnInit() || !WebPier::CheckModule())
            return false;

        wxImage::AddHandler(new wxPNGHandler);

        wxIcon icon;
        icon.CopyFromBitmap(::GetLogo());

        m_checker = new wxSingleInstanceChecker(wxApp::GetAppName() + '-' + wxGetUserId(), wxStandardPaths::Get().GetTempDir());
        if (m_checker->IsAnotherRunning())
        {
            delete m_checker;
            m_checker = nullptr;

            return (bool)CreateMainFrame(icon);
        }

        return (bool)new CTaskBarIcon(icon);
    }

    int OnExit() override
    {
        delete m_checker;
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(CWebPierApp);
