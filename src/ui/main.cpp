#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/notifmsg.h>
#include <wx/stdpaths.h>
#include <ui/mainframe.h>
#include <wx/mstream.h>
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

CMainFrame* CreateMainFrame(wxTaskBarIcon* taskBar = nullptr)
{
    CMainFrame* frame = new CMainFrame(taskBar);

    frame->Populate();
    frame->Show(true);

    return frame;
}

constexpr const char* IPCTaskbarSocket = "taskbar.jack";
constexpr const char* IPCTaskbarHost = "localhost";
constexpr const char* IPCTaskbarTopic = "webpier";
constexpr const char* IPCTaskbarCommand = "raise";

class CTaskBarConnection : public wxConnection
{
    CMainFrame* m_frame;

public:

    CTaskBarConnection(CMainFrame* frame) : m_frame(frame)
    {
    }

protected:

    bool OnExec(const wxString& topic, const wxString& command) override
    {
        if (topic == IPCTaskbarTopic && command == IPCTaskbarCommand)
        {
            m_frame->Show(true);
            m_frame->Iconize(false);
            m_frame->Raise();
            return true;
        }
        return false;
    }
};

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
    }

    ~CTaskBarIcon() override
    {
        if (m_frame)
            delete m_frame;
    }

    bool Init()
    {
        if (wxServer::Create(WebPier::GetHome() + "/" + IPCTaskbarSocket))
        {
            m_frame = CreateMainFrame(this);
            m_frame->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(CTaskBarIcon::OnFrameClose), NULL, this);
            return true;
        }
        return false;
    }

    wxConnectionBase* OnAcceptConnection(const wxString& topic)
    {
        return new CTaskBarConnection(m_frame);
    }

protected:

    void OnLeftButtonClick(wxTaskBarIconEvent&)
    {
        m_frame->Show(true);
        m_frame->Iconize(false);
        m_frame->Raise();
    }

    void OnMenuConfigure(wxCommandEvent&)
    {
        m_frame->Show(true);
        m_frame->Iconize(false);
        m_frame->Raise();
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

class CWebPierApp : public wxApp
{
public:
    bool OnInit() override
    {
#ifdef wxUSE_LOG
        wxLog::SetActiveTarget(new wxLogStderr());
#endif

        wxClient* client = new wxClient();
        wxConnectionBase* connect = client->MakeConnection(IPCTaskbarHost, WebPier::GetHome() + "/" + IPCTaskbarSocket, IPCTaskbarTopic);

        if (connect && connect->Execute(IPCTaskbarCommand))
        {
            delete connect;
            delete client;

            return false;
        }

        delete connect;
        delete client;

        wxInitAllImageHandlers();

        CTaskBarIcon* taskbar = new CTaskBarIcon();
        return wxApp::OnInit() && WebPier::Init() && taskbar->Init();
    }
};

wxIMPLEMENT_APP(CWebPierApp);
