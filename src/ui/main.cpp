#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/notifmsg.h>
#include <wx/stdpaths.h>
#include <ui/mainframe.h>
#include <wx/mstream.h>
#include <wx/cmdline.h>
#include <wx/ipc.h>
#include <wx/log.h>

#ifndef WIN32
#include <assets/logo.h>
#include <assets/blue.h>
#include <assets/green.h>
#include <assets/red.h>
#include <assets/grey.h>
#include <assets/add.h>
#include <assets/edit.h>
#include <assets/delete.h>
#endif

const wxIconBundle& GetAppIconBundle()
{
#ifdef WIN32
    static const wxIconBundle s_icon(wxT("IDI_ICON"), (WXHINSTANCE)0);
#else
    static const wxIconBundle s_icon([]()
    {
        wxMemoryInputStream stream(webpier_logo_ico, sizeof(webpier_logo_ico));
        return wxIconBundle(stream);
    }());
#endif
    return s_icon;
}

const wxBitmap& GetBlueCircleImage()
{
#ifdef WIN32
    static const wxBitmap s_image(wxBITMAP_PNG(IDB_BLUE));
#else
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(blue_circle));
#endif
    return s_image;
}

const wxBitmap& GetGreenCircleImage()
{
#ifdef WIN32
    static const wxBitmap s_image(wxBITMAP_PNG(IDB_GREEN));
#else
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(green_circle));
#endif
    return s_image;
}

const wxBitmap& GetRedCircleImage()
{
#ifdef WIN32
    static const wxBitmap s_image(wxBITMAP_PNG(IDB_RED));
#else
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(red_circle));
#endif
    return s_image;
}

const wxBitmap& GetGreyCircleImage()
{
#ifdef WIN32
    static const wxBitmap s_image(wxBITMAP_PNG(IDB_GREY));
#else
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(grey_circle));
#endif
    return s_image;
}

const wxBitmap& GetAddBtnImage()
{
#ifdef WIN32
    static const wxBitmap s_image(wxBITMAP_PNG(IDB_ADD));
#else
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(add_btn));
#endif
    return s_image;
}

const wxBitmap& GetEditBtnImage()
{
#ifdef WIN32
    static const wxBitmap s_image(wxBITMAP_PNG(IDB_EDIT));
#else
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(edit_btn));
#endif
    return s_image;
}

const wxBitmap& GetDeleteBtnImage()
{
#ifdef WIN32
    static const wxBitmap s_image(wxBITMAP_PNG(IDB_DELETE));
#else
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(delete_btn));
#endif
    return s_image;
}

class CTaskBarIcon : public wxTaskBarIcon, public wxServer
{
    CMainFrame* m_frame = nullptr;

public:

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    CTaskBarIcon(wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE)
        : wxTaskBarIcon(iconType)
#else
    CTaskBarIcon()
#endif
    {
        this->SetIcon(wxBitmapBundle::FromIconBundle(::GetAppIconBundle()));

        m_frame = new CMainFrame(this);
        m_frame->Populate();
        m_frame->Show(false);
        m_frame->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(CTaskBarIcon::OnFrameClose), NULL, this);
    }

    ~CTaskBarIcon() override
    {
        if (m_frame)
            delete m_frame;
    }

    void Raise()
    {
        m_frame->Show(true);
        m_frame->Iconize(false);
        m_frame->Raise();
    }

protected:

    void OnLeftButtonClick(wxTaskBarIconEvent&)
    {
        Raise();
    }

    void OnMenuConfigure(wxCommandEvent&)
    {
        Raise();
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
            auto status = WebPier::Backend::Status();

            for (const auto& item : status)
            {
                wxMenu* menu = item.Pier == pier ? exports : imports;
                wxMenuItem* check = menu->AppendCheckItem(wxID_ANY, item.Pier + wxT(":") + item.Service);

                if (item.State == WebPier::Backend::Health::Asleep)
                {
                    check->Check(false);
                    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this, item](wxCommandEvent&)
                    {
                        try
                        {
                            WebPier::Backend::Engage(item);
                        }
                        catch(const std::exception& ex)
                        {
                            wxNotificationMessage msg(wxT("WebPier"), _("Can't start ") + item.Pier + wxT(":") + item.Service + _(" service. ") + ex.what(), nullptr, wxICON_ERROR);
#if defined(__WXMSW__) && defined(wxHAS_NATIVE_NOTIFICATION_MESSAGE)
                            msg.UseTaskBarIcon(this);
#endif
                            msg.Show(10);
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
                            WebPier::Backend::Unplug(item);
                        }
                        catch(const std::exception& ex)
                        {
                            wxNotificationMessage msg(wxT("WebPier"), _("Can't stop ") + item.Pier + wxT(":") + item.Service + _(" service. ") + ex.what(), nullptr, wxICON_ERROR);
#if defined(__WXMSW__) && defined(wxHAS_NATIVE_NOTIFICATION_MESSAGE)
                            msg.UseTaskBarIcon(this);
#endif
                            msg.Show(10);
                        }
                        m_frame->RefreshStatus(item);
                    }, check->GetId());
                }
            }
        }
        catch(const std::exception& ex)
        {
            wxNotificationMessage msg(wxT("WebPier"), _("Can't popup menu. ") + ex.what(), nullptr, wxICON_ERROR);
#if defined(__WXMSW__) && defined(wxHAS_NATIVE_NOTIFICATION_MESSAGE)
            msg.UseTaskBarIcon(this);
#endif
            msg.Show(10);
        }

        menu->Append(wxID_ANY, "&Import", imports);
        menu->Append(wxID_ANY, "&Export", exports);

        wxMenuItem* unplug = menu->Append(wxID_ANY, _("&Unplug"));
        wxMenuItem* reboot = menu->Append(wxID_ANY, _("&Reboot"));

        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&)
        {
            try
            {
                WebPier::Backend::Unplug();
            }
            catch(const std::exception& ex)
            {
                wxNotificationMessage msg(wxT("WebPier"), _("Can't unplug the pier. ") + ex.what(), nullptr, wxICON_ERROR);
#if defined(__WXMSW__) && defined(wxHAS_NATIVE_NOTIFICATION_MESSAGE)
                msg.UseTaskBarIcon(this);
#endif
                msg.Show(10);
            }
            m_frame->RefreshStatus();
        }, unplug->GetId());

        menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent&)
        {
            try
            {
                WebPier::Backend::Engage();
            }
            catch(const std::exception& ex)
            {
                wxNotificationMessage msg(wxT("WebPier"), _("Can't reboot the pier. ") + ex.what(), nullptr, wxICON_ERROR);
#if defined(__WXMSW__) && defined(wxHAS_NATIVE_NOTIFICATION_MESSAGE)
                msg.UseTaskBarIcon(this);
#endif
                msg.Show(10);
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
    EVT_TASKBAR_LEFT_UP(CTaskBarIcon::OnLeftButtonClick)
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLeftButtonClick)
wxEND_EVENT_TABLE()

class CTaskBarConnection : public wxConnection
{
    CTaskBarIcon* m_taskbar = nullptr;

public:

    CTaskBarConnection(CTaskBarIcon* taskbar) : m_taskbar(taskbar)
    {
    }

protected:

    bool OnExec(const wxString& topic, const wxString& command) override
    {
        if (topic == "webpier" && command == "raise")
        {
            m_taskbar->Raise();
            return true;
        }
        return false;
    }
};

class CWebPierApp : public wxApp, public wxServer
{
    bool m_tray;
    wxString m_home;
    CTaskBarIcon* m_taskbar = nullptr;

protected:

    wxConnectionBase* OnAcceptConnection(const wxString& topic) override
    {
        return new CTaskBarConnection(m_taskbar);
    }

public:

    void OnInitCmdLine(wxCmdLineParser& parser) override
    {
        static const wxCmdLineEntryDesc s_cmdLineDesc[] =
        {
            { wxCMD_LINE_PARAM, NULL, NULL, "context directory", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
            { wxCMD_LINE_SWITCH, "t", "tray", "start app in tray" },
            { wxCMD_LINE_NONE }
        };

        parser.SetDesc(s_cmdLineDesc);
        parser.SetSwitchChars(wxT("-"));
    }

    bool OnCmdLineParsed(wxCmdLineParser& parser) override
    {
        m_tray = parser.Found(wxT("t"));

        if (!wxGetEnv("WEBPIER_HOME", &m_home))
            m_home = wxStandardPaths::Get().GetUserLocalDataDir();

        for (size_t i = 0; i < parser.GetParamCount(); ++i)
        {
            m_home = parser.GetParam(i);
            break;
        }
        return true;
    }

    bool OnInit() override
    {
#ifdef wxUSE_LOG
        wxLog::SetActiveTarget(new wxLogStderr());
#endif
        if (wxApp::OnInit())
        {
            wxString socket = wxStandardPaths::Get().GetTempDir() + "/" + WebPier::Utils::MakeTextHash(m_home) + ".webpier";

            wxClient* client = new wxClient();
            wxConnectionBase* connect = client->MakeConnection("localhost", socket, "webpier");

            if (connect)
            {
                if (!m_tray)
                    connect->Execute("raise");

                delete connect;
                delete client;

                return false;
            }

            delete connect;
            delete client;

            wxInitAllImageHandlers();

            if (wxServer::Create(socket) && WebPier::Init(m_home))
            {
                m_taskbar = new CTaskBarIcon();

                if (!m_tray)
                    m_taskbar->Raise();

                return true;
            }
        }
        return false;
    }
};

wxIMPLEMENT_APP(CWebPierApp);
