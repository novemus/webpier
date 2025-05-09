#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/notifmsg.h>
#include <wx/snglinst.h>
#include <wx/stdpaths.h>
#include <ui/mainframe.h>
#include <wx/mstream.h>

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

class CTaskBarIcon : public wxTaskBarIcon
{
    CMainFrame* m_frame;

public:

#if defined(__WXOSX__) && wxOSX_USE_COCOA
    CTaskBarIcon(wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE)
        : wxTaskBarIcon(iconType)
#else
    CTaskBarIcon()
#endif
    {
        this->SetIcon(wxBitmapBundle::FromIconBundle(::GetAppIconBundle()));

        m_frame = CreateMainFrame(this);
        m_frame->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(CTaskBarIcon::OnFrameClose), NULL, this);
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
    EVT_TASKBAR_LEFT_DCLICK(CTaskBarIcon::OnLeftButtonDClick)
wxEND_EVENT_TABLE()

class CWebPierApp : public wxApp
{
    wxSingleInstanceChecker* m_checker;

public:

    bool OnInit() override
    {
        wxInitAllImageHandlers();

        if (!wxApp::OnInit() || !WebPier::Init())
            return false;

        m_checker = new wxSingleInstanceChecker(wxApp::GetAppName() + '-' + wxGetUserId(), wxStandardPaths::Get().GetTempDir());
        if (m_checker->IsAnotherRunning())
        {
            delete m_checker;
            m_checker = nullptr;

            return (bool)CreateMainFrame();
        }

        return (bool)new CTaskBarIcon();
    }

    int OnExit() override
    {
        delete m_checker;
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(CWebPierApp);
