#include "context.h"
#include "startupdialog.h"
#include "messagedialog.h"
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include <wx/utils.h> 
#include <wx/stdpaths.h>
#include <wx/dataview.h>
#include <wx/regex.h>

static const unsigned char blue_circle_png[] = {
0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 
0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x08, 0x06, 0x00, 0x00, 0x00, 0xE0, 0x77, 0x3D, 
0xF8, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x10, 0xDF, 0x00, 0x00, 0x10, 
0xDF, 0x01, 0x74, 0x55, 0x5E, 0x30, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6F, 
0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2E, 0x69, 0x6E, 0x6B, 0x73, 0x63, 
0x61, 0x70, 0x65, 0x2E, 0x6F, 0x72, 0x67, 0x9B, 0xEE, 0x3C, 0x1A, 0x00, 0x00, 0x00, 0xBD, 0x49, 
0x44, 0x41, 0x54, 0x48, 0x89, 0xED, 0x93, 0x31, 0x0E, 0x82, 0x40, 0x10, 0x45, 0xDF, 0x20, 0x27, 
0xB2, 0x10, 0x0F, 0xA1, 0xE7, 0xB0, 0xA0, 0x27, 0x31, 0x58, 0xA2, 0x85, 0xB5, 0x95, 0xE7, 0xD0, 
0x78, 0x05, 0x69, 0xF0, 0x42, 0x0B, 0xDF, 0x02, 0x6C, 0x14, 0x17, 0x45, 0x0B, 0x4D, 0x78, 0xED, 
0xCC, 0xFC, 0xB7, 0x99, 0xDD, 0x85, 0x81, 0x81, 0x9F, 0xC7, 0x5E, 0x69, 0x9A, 0x6D, 0x35, 0xB6, 
0x8A, 0x54, 0x30, 0x6D, 0x86, 0xCE, 0x0A, 0x58, 0x1F, 0x12, 0x2B, 0x3E, 0x16, 0xCC, 0x37, 0x5A, 
0x28, 0x60, 0x07, 0x84, 0x77, 0x25, 0x27, 0x11, 0x1F, 0x97, 0xB6, 0xEF, 0x2D, 0x98, 0x6D, 0x35, 
0xA6, 0x22, 0x07, 0x46, 0x2D, 0xBD, 0x02, 0xCA, 0x52, 0x4C, 0x4E, 0x4B, 0xBB, 0x3C, 0xCB, 0x08, 
0xBC, 0xF6, 0x8A, 0x94, 0xFA, 0xE4, 0x6D, 0x07, 0x31, 0x20, 0x0C, 0x8D, 0x95, 0x2F, 0xC3, 0x2B, 
0xB8, 0xED, 0xBC, 0xA3, 0x27, 0xEA, 0x2D, 0xF8, 0x06, 0xFE, 0x15, 0xC1, 0xB9, 0x2B, 0xC0, 0x20, 
0xEF, 0x2D, 0x50, 0xC0, 0x1A, 0x70, 0xD4, 0x17, 0xFA, 0x50, 0x46, 0x38, 0x27, 0xB2, 0xDE, 0x82, 
0x43, 0x62, 0x85, 0x44, 0x0C, 0x94, 0x2D, 0xE5, 0x52, 0x10, 0xFB, 0x5E, 0x10, 0xBC, 0xFF, 0xD1, 
0xA2, 0x66, 0x28, 0x77, 0x22, 0xEB, 0x0A, 0x1F, 0x18, 0xF8, 0x13, 0xAE, 0x44, 0x57, 0x3E, 0xD6, 
0x7A, 0x0D, 0xEE, 0x81, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};

const wxBitmap& GetBlueCircleImage()
{
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(blue_circle));
    return s_image;
}

static const unsigned char green_circle_png[] = {
0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 
0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x08, 0x06, 0x00, 0x00, 0x00, 0xE0, 0x77, 0x3D, 
0xF8, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x10, 0xDF, 0x00, 0x00, 0x10, 
0xDF, 0x01, 0x74, 0x55, 0x5E, 0x30, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6F, 
0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2E, 0x69, 0x6E, 0x6B, 0x73, 0x63, 
0x61, 0x70, 0x65, 0x2E, 0x6F, 0x72, 0x67, 0x9B, 0xEE, 0x3C, 0x1A, 0x00, 0x00, 0x00, 0x89, 0x49, 
0x44, 0x41, 0x54, 0x48, 0x89, 0xED, 0x93, 0xB1, 0x11, 0x83, 0x40, 0x0C, 0x04, 0x17, 0x2A, 0x72, 
0x07, 0x94, 0x61, 0xB7, 0x40, 0x25, 0x50, 0x81, 0xDD, 0x07, 0x34, 0xE2, 0x2A, 0x20, 0x40, 0xB4, 
0x60, 0x11, 0xF0, 0x0E, 0xFF, 0xE6, 0x07, 0xC3, 0x38, 0xD1, 0xCE, 0x5C, 0xF4, 0x37, 0x5A, 0x05, 
0x2F, 0x08, 0x82, 0xA0, 0x8C, 0x85, 0x06, 0x63, 0xC4, 0x98, 0x53, 0x46, 0x16, 0x9A, 0x73, 0x86, 
0x1B, 0x3D, 0x86, 0x67, 0xD2, 0xFD, 0xBA, 0xF9, 0x03, 0xC3, 0x59, 0xF9, 0x64, 0x25, 0x2B, 0x77, 
0x35, 0xA2, 0x96, 0x82, 0x9A, 0x16, 0x00, 0xA7, 0xCA, 0x76, 0x3C, 0x75, 0x0E, 0x09, 0x9C, 0x9B, 
0x7C, 0xDF, 0x91, 0x1D, 0x2D, 0xA8, 0x78, 0x17, 0x08, 0x64, 0x47, 0x0B, 0xE0, 0x99, 0x44, 0x2E, 
0x96, 0x78, 0x15, 0x2C, 0x21, 0x30, 0xBA, 0xEB, 0x7E, 0xD1, 0x97, 0xFD, 0x0E, 0x06, 0x8C, 0x29, 
0x65, 0x38, 0xEF, 0x0E, 0x82, 0xE0, 0xEF, 0x6C, 0x26, 0xEE, 0x49, 0x26, 0x23, 0x6F, 0x27, 0x25, 
0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};

const wxBitmap& GetGreenCircleImage()
{
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(green_circle));
    return s_image;
}

static const unsigned char red_circle_png[] = {
0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 
0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x08, 0x06, 0x00, 0x00, 0x00, 0xE0, 0x77, 0x3D, 
0xF8, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x10, 0xDF, 0x00, 0x00, 0x10, 
0xDF, 0x01, 0x74, 0x55, 0x5E, 0x30, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6F, 
0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2E, 0x69, 0x6E, 0x6B, 0x73, 0x63, 
0x61, 0x70, 0x65, 0x2E, 0x6F, 0x72, 0x67, 0x9B, 0xEE, 0x3C, 0x1A, 0x00, 0x00, 0x00, 0x73, 0x49, 
0x44, 0x41, 0x54, 0x48, 0x89, 0xED, 0x92, 0x51, 0x0A, 0x80, 0x20, 0x10, 0x05, 0x87, 0x6E, 0x94, 
0x87, 0xAA, 0xD3, 0xED, 0x3D, 0xEA, 0x1E, 0x7A, 0x0A, 0xA1, 0xD7, 0x8F, 0x5F, 0x22, 0x25, 0x69, 
0x10, 0xE1, 0xC0, 0xFB, 0x5A, 0x78, 0xB3, 0x8B, 0xC2, 0x60, 0xF0, 0x0F, 0x04, 0x4E, 0x60, 0x82, 
0x90, 0x62, 0x02, 0xD7, 0xAB, 0x7C, 0x15, 0x44, 0x81, 0xB2, 0x44, 0xC1, 0xD2, 0x63, 0xF3, 0x28, 
0x38, 0x0A, 0x82, 0x23, 0xCD, 0xE6, 0x16, 0x81, 0x15, 0x8A, 0xF3, 0x58, 0x8B, 0x20, 0x54, 0x08, 
0xFC, 0x55, 0xC7, 0xF4, 0xD8, 0x5E, 0xC9, 0x9D, 0x60, 0xAB, 0xE8, 0xD8, 0x1F, 0xDB, 0x5F, 0x7F, 
0xE4, 0x24, 0x59, 0x5E, 0xFB, 0xA6, 0xD9, 0x25, 0x26, 0xF0, 0x29, 0xD6, 0xBC, 0xF9, 0x60, 0xF0, 
0x1D, 0x4E, 0xF8, 0x8B, 0x7D, 0x91, 0x71, 0x04, 0x82, 0x87, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 
0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
};

const wxBitmap& GetRedCircleImage()
{
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(red_circle));
    return s_image;
}

static const unsigned char grey_circle_png[] = {
0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 
0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x08, 0x06, 0x00, 0x00, 0x00, 0xE0, 0x77, 0x3D, 
0xF8, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x10, 0xDF, 0x00, 0x00, 0x10, 
0xDF, 0x01, 0x74, 0x55, 0x5E, 0x30, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6F, 
0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x77, 0x77, 0x77, 0x2E, 0x69, 0x6E, 0x6B, 0x73, 0x63, 
0x61, 0x70, 0x65, 0x2E, 0x6F, 0x72, 0x67, 0x9B, 0xEE, 0x3C, 0x1A, 0x00, 0x00, 0x00, 0xD1, 0x49, 
0x44, 0x41, 0x54, 0x48, 0x89, 0xED, 0x92, 0x31, 0x6E, 0xC2, 0x40, 0x10, 0x45, 0xDF, 0x0C, 0xAE, 
0xD3, 0xE5, 0x22, 0x14, 0x89, 0xCB, 0x6D, 0x73, 0x08, 0xEA, 0x14, 0x1C, 0x01, 0x64, 0x8F, 0xA5, 
0xF8, 0x06, 0xA9, 0xB8, 0x07, 0x1D, 0x85, 0x4B, 0x28, 0x48, 0x8E, 0x81, 0xA8, 0x69, 0xC1, 0x93, 
0x02, 0x57, 0x91, 0xED, 0x8D, 0xEC, 0x26, 0x48, 0x7E, 0xE5, 0xFE, 0xD1, 0x7F, 0xA3, 0xD1, 0xC2, 
0xC4, 0xC4, 0xBF, 0x47, 0xFE, 0x32, 0x64, 0x66, 0x73, 0x60, 0x05, 0xA4, 0xCD, 0xD3, 0x1E, 0x28, 
0xCD, 0xEC, 0x6B, 0xB4, 0xA0, 0x28, 0x8A, 0x77, 0x77, 0xFF, 0x04, 0x92, 0x5F, 0xD1, 0x15, 0x58, 
0x9A, 0xD9, 0x66, 0xB0, 0xA0, 0xD9, 0xFC, 0x00, 0xCC, 0x5A, 0x66, 0x1D, 0xB8, 0xA9, 0xEA, 0x4B, 
0x96, 0x65, 0xDF, 0x5D, 0x1D, 0xDA, 0x27, 0xE0, 0x7E, 0x96, 0xA4, 0x63, 0x11, 0x01, 0x92, 0xBA, 
0xAE, 0xD7, 0x7D, 0x05, 0x31, 0x41, 0x1A, 0xC9, 0x71, 0xF7, 0xD7, 0x31, 0x82, 0xD1, 0xC4, 0x04, 
0xFB, 0x58, 0x81, 0x88, 0x1C, 0xC6, 0x08, 0x4A, 0xEE, 0xBF, 0xC5, 0x5B, 0x32, 0x77, 0xF7, 0xAB, 
0xAA, 0x7E, 0xF4, 0x15, 0xCC, 0xFA, 0xC2, 0xAA, 0xAA, 0x4E, 0x21, 0x84, 0x33, 0xF0, 0xD6, 0xB2, 
0xCC, 0x4D, 0x44, 0x96, 0x79, 0x9E, 0x6F, 0x07, 0x0B, 0x1A, 0xC9, 0x31, 0x84, 0xB0, 0x05, 0x9E, 
0xDD, 0xFD, 0x09, 0xB8, 0x88, 0xC8, 0x4E, 0x55, 0x17, 0xB1, 0xF2, 0x89, 0x89, 0x07, 0xE1, 0x07, 
0x2C, 0x3F, 0x3B, 0xB2, 0xA2, 0x92, 0xFD, 0x18, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 
0xAE, 0x42, 0x60, 0x82
};

const wxBitmap& GetGreyCircleImage()
{
    static const wxBitmap s_image(wxBITMAP_PNG_FROM_DATA(grey_circle));
    return s_image;
}

namespace WebPier
{
    class Context
    {
        static std::shared_ptr<webpier::context>& getImpl()
        {
            static std::shared_ptr<webpier::context> s_context;
            return s_context;
        }

        static wxString getHome()
        {
            wxString home;
            if (!wxGetEnv("WEBPIER_HOME", &home))
                home = wxStandardPaths::Get().GetUserLocalDataDir();

            if (!wxFileName::Exists(home) && !wxFileName::Mkdir(home))
            {
                CMessageDialog dialog(nullptr, _("Can't create home directory ") + home, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
                dialog.ShowModal();
                throw webpier::usage_error("no context");
            }

            return home;
        }

        static wxString readRoute(const wxString& home)
        {
            wxString host;
            wxString route = home + "/webpier";

            if (wxFileName::Exists(route))
            {
                wxFile in(route, wxFile::read);
                if (!in.ReadAll(&host))
                {
                    CMessageDialog dialog(nullptr, _("Can't read context reference ") + route, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
                    dialog.ShowModal();
                    throw webpier::usage_error("no context");
                }
            }

            return host;
        }

        static void writeRoute(const wxString& home, const wxString& host)
        {
            wxString route = home + "/webpier";

            wxFile out(route, wxFile::write);
            if (!out.Write(host))
            {
                CMessageDialog dialog(nullptr, _("Can't write context reference ") + route, wxDEFAULT_DIALOG_STYLE|wxICON_ERROR);
                dialog.ShowModal();
                throw webpier::usage_error("no context");
            }
        }

    public:

        static std::shared_ptr<webpier::context> Get()
        {
            auto& impl = getImpl();
            if (impl)
                return impl;

            wxString home = getHome();
            wxString host = readRoute(home);

            if (host.IsEmpty())
            {
                CStartupDialog dialog(nullptr);
                if (dialog.ShowModal() == wxID_OK)
                {
                    host = dialog.GetIdentity();
                    writeRoute(home, host);
                }
            }

            if (!host.IsEmpty())
                impl = webpier::open_context(home.ToStdString(), host.ToStdString());

            if (!impl)
                throw webpier::usage_error("no context");

            return impl;
        }

        static void Switch(const wxString& host, bool tidy = false)
        {
            auto& impl = getImpl();

            if (impl && impl->get_host() == host)
               return;
            
            if (impl && tidy)
                wxFileName::Rmdir(impl->get_home(), wxPATH_RMDIR_FULL|wxPATH_RMDIR_RECURSIVE);

            impl.reset();

            writeRoute(getHome(), host);
        }
    };

    void Service::Store() noexcept(false)
    {
        if (!IsDirty())
            return;

        auto context = Context::Get();
        if (m_local)
        {
            if (!m_origin.id.empty())
                context->del_local_service(m_origin.id);
            context->add_local_service(m_actual);
        }
        else
        {
            if (!m_origin.id.empty() && !m_origin.peer.empty())
                context->del_remote_service(m_origin.peer, m_origin.id);
            context->add_remote_service(m_actual);
        }
        m_origin = m_actual;
    }

    void Service::Purge() noexcept(false)
    {
        auto context = Context::Get();
        if (m_local)
        {
            if (!m_origin.id.empty())
                context->del_local_service(m_origin.id);
        }
        else
        {
            if (!m_origin.id.empty() && !m_origin.peer.empty())
                context->del_remote_service(m_origin.peer, m_origin.id);
        }
        m_origin = {};
    }

    wxVector<wxVariant> Service::ToListView() const noexcept(true)
    {
        wxVector<wxVariant> data;
        data.push_back(wxVariant(wxDataViewIconText(m_actual.id, ::GetGreyCircleImage())));
        data.push_back(wxVariant(wxString(m_actual.peer)));
        data.push_back(wxVariant(wxString(m_actual.service)));
        data.push_back(wxVariant(wxString(m_actual.gateway)));
        data.push_back(wxVariant(wxString(m_actual.rendezvous.bootstrap.empty() ? wxT("Email") : wxT("DHT"))));
        data.push_back(wxVariant(wxString(m_actual.autostart ? _("yes") : _("no"))));
        return data;
    }

    wxArrayString Service::GetPees() const noexcept(true)
    {
        wxArrayString array;
        std::istringstream stream(m_actual.peer);
        std::string line;    
        while (std::getline(stream, line, ' '))
            array.Add(line);
        return array;
    }

    void Service::SetPeers(const wxArrayString& peers) noexcept(true)
    {
        m_actual.peer.clear();
        for (size_t i = 0; i < peers.Count(); ++i)
        {
            m_actual.peer += peers[i].ToStdString();
            if (i < peers.Count() - 1)
                m_actual.peer += " ";
        }
    }

    void Service::AddPeer(const wxString& peer) noexcept(true)
    {
        if (m_local)
        {
            auto beg = m_actual.peer.find(peer.ToStdString());
            if (beg != std::string::npos)
            {
                auto end = beg + peer.size();
                bool begGap = beg == 0 || m_actual.peer[beg - 1] == ' ';
                bool endGap = end == m_actual.peer.size() || m_actual.peer[end] == ' ';
                if (begGap && endGap)
                    return;
            }

            if (!m_actual.peer.empty())
                m_actual.peer += " ";

            m_actual.peer += peer;
        }
        else
            m_actual.peer = peer;
    }

    void Service::DelPeer(const wxString& peer) noexcept(true)
    {
        auto beg = m_actual.peer.find(peer.ToStdString());
        if (beg != std::string::npos)
        {
            auto end = beg + peer.size();
            bool begGap = beg == 0 || m_actual.peer[beg - 1] == ' ';
            bool endGap = end == m_actual.peer.size() || m_actual.peer[end] == ' ';
            if (begGap && endGap)
            {
                if (end < m_actual.peer.size())
                    ++end;

                m_actual.peer.erase(beg, end - beg);
            }
        }
    }

    bool Service::IsPeerPresent(const wxString& peer) const noexcept(true)
    {
        if (m_local)
        {
            auto beg = m_actual.peer.find(peer.ToStdString());
            if (beg != std::string::npos)
            {
                auto end = beg + peer.size();
                bool begGap = beg == 0 || m_actual.peer[beg - 1] == ' ';
                bool endGap = end == m_actual.peer.size() || m_actual.peer[end] == ' ';
                if (begGap && endGap)
                    return true;
            }
            return false;
        }

        return m_actual.peer == peer.ToStdString();
    }

    void Service::UseDhtRendezvous() noexcept(true)
    { 
        if (!m_origin.rendezvous.bootstrap.empty())
        {
            m_actual.rendezvous = m_origin.rendezvous;
            return;
        }
        auto config = GetConfig();
        m_actual.rendezvous.bootstrap = config.GetDhtBootstrap();
        m_actual.rendezvous.network = config.GetDhtNetwork(); 
    }

    void Service::UseEmailRendezvous() noexcept(true)
    {
        m_actual.rendezvous.bootstrap.clear();
        m_actual.rendezvous.network = 0;
    }

    bool Service::IsEqual(const Service& other) const noexcept(true)
    {
        return m_actual.id == other.m_actual.id
            && m_actual.peer == other.m_actual.peer
            && m_actual.gateway == other.m_actual.gateway
            && m_actual.service == other.m_actual.service
            && m_actual.autostart == other.m_actual.autostart
            && m_actual.obscure == other.m_actual.obscure
            && m_actual.rendezvous.bootstrap == other.m_actual.rendezvous.bootstrap
            && m_actual.rendezvous.network == other.m_actual.rendezvous.network;
    }

    bool Service::IsDirty() const noexcept(true)
    {
        return m_actual.id != m_origin.id
            || m_actual.peer == m_origin.peer
            || m_actual.gateway == m_origin.gateway
            || m_actual.service == m_origin.service
            || m_actual.autostart == m_origin.autostart
            || m_actual.obscure == m_origin.obscure
            || m_actual.rendezvous.bootstrap == m_origin.rendezvous.bootstrap
            || m_actual.rendezvous.network == m_origin.rendezvous.network;
    }

    void Config::Store(bool tidy) noexcept(false)
    {
        Context::Switch(m_actual.host, tidy);
        Context::Get()->set_config(m_actual);
    }

    const wxRegEx IDENTITY_PATTERN("([^/]+)/([^/]+)");

    wxString Config::GetOwner() const noexcept(true)
    {
        return IDENTITY_PATTERN.Matches(m_actual.host) ? IDENTITY_PATTERN.GetMatch(m_actual.host, 1) : m_actual.host;
    }

    wxString Config::GetPier() const noexcept(true)
    {
        return IDENTITY_PATTERN.Matches(m_actual.host) ? IDENTITY_PATTERN.GetMatch(m_actual.host, 2) : m_actual.host;
    }

    wxString GetHost() noexcept(false)
    {
        return Context::Get()->get_host();
    }

    Config GetConfig() noexcept(false)
    {
        webpier::config config;
        Context::Get()->get_config(config);
        return Config(config);
    }

    ServiceList GetLocalServices() noexcept(false)
    {
        ServiceList collection;
        std::vector<webpier::service> list;
        Context::Get()->get_local_services(list);
        for (const auto& item : list)
        {
            ServicePtr ptr(new Service(true, item));
            collection[wxUIntPtr(ptr.get())] = ptr;
        }
        return collection;
    }

    ServiceList GetRemoteServices() noexcept(false)
    {
        ServiceList collection;
        std::vector<webpier::service> list;
        Context::Get()->get_remote_services(list);
        for (const auto& item : list)
        {
            ServicePtr ptr(new Service(false, item));
            collection[wxUIntPtr(ptr.get())] = ptr;
        }

        return collection;
    }

    ServicePtr GetLocalService(const wxString& id) noexcept(false)
    {
        webpier::service raw;
        if (Context::Get()->get_local_service(id.ToStdString(), raw))
            return ServicePtr(new Service(true, raw));
        return ServicePtr();
    }

    ServicePtr GetRemoteService(const wxString& peer, const wxString& id) noexcept(false)
    {
        webpier::service raw;
        if (Context::Get()->get_remote_service(peer.ToStdString(), id.ToStdString(), raw))
            return ServicePtr(new Service(false, raw));
        return ServicePtr();
    }

    wxArrayString GetPeers() noexcept(false)
    {
        wxArrayString array;
        std::vector<std::string> list;
        Context::Get()->get_peers(list);
        for (const auto& item : list)
            array.Add(item);
        return array;
    }

    bool IsUselessPeer(const wxString& id) noexcept(false)
    {
        auto isUsedForRemote = [&]()
        {
            std::vector<webpier::service> list;
            Context::Get()->get_remote_services(list);
            auto iter = std::find_if(list.begin(), list.end(), [&id](const auto& item)
            {
                return item.peer == id;
            });
            return iter != list.end();
        };

        auto isUsedForLocal = [&]()
        {
            std::vector<webpier::service> list;
            Context::Get()->get_local_services(list);
            auto iter = std::find_if(list.begin(), list.end(), [peer = id.ToStdString()](const auto& item)
            {
                return item.peer.find(peer) != std::string::npos;
            });
            return iter != list.end();
        };

        return !isUsedForRemote() && !isUsedForLocal();
    }

    bool IsPeerExist(const wxString& id) noexcept(false)
    {
        return Context::Get()->is_peer_exist(id.ToStdString());
    }

    void AddPeer(const wxString& id, const wxString& cert) noexcept(false)
    {
        Context::Get()->add_peer(id.ToStdString(), cert.ToStdString());
    }

    void DelPeer(const wxString& id) noexcept(false)
    {
        Context::Get()->del_peer(id.ToStdString());
    }

    wxString GetCertificate(const wxString& id) noexcept(false)
    {
        return Context::Get()->get_certificate(id.ToStdString());
    }

    wxString GetFingerprint(const wxString& id) noexcept(false)
    {
        return Context::Get()->get_fingerprint(id.ToStdString());
    }

    void WriteExchangeFile(const wxString& file, const Exchange& data) noexcept(false)
    {
        boost::property_tree::ptree doc;
        doc.put("pier", data.pier.ToStdString());
        doc.put("certificate", data.certificate.ToStdString());

        boost::property_tree::ptree array;
        for (const auto& pair : data.services)
        {
            boost::property_tree::ptree item;
            item.put("id", pair.second->GetId().ToStdString());
            item.put("obscure", pair.second->IsObscure());
            item.put("rendezvous.dht.bootstrap", pair.second->GetDhtBootstrap().ToStdString());
            item.put("rendezvous.dht.network", pair.second->GetDhtNetwork());
            array.push_back(std::make_pair("", item));
        }
        doc.put_child("services", array);

        boost::property_tree::write_json(file.ToStdString(), doc);
    }

    void ReadExchangeFile(const wxString& file, Exchange& data) noexcept(false)
    {
        boost::property_tree::ptree doc;
        boost::property_tree::read_json(file.ToStdString(), doc);

        data.pier = doc.get<std::string>("pier", "");
        data.certificate = doc.get<std::string>("certificate", "");

        boost::property_tree::ptree array;
        for (auto& item : doc.get_child("services", array))
        {
            ServicePtr service(new Service(false));
            service->SetId(item.second.get<std::string>("id"));
            service->SetPeer(data.pier);
            service->SetObscure(item.second.get<bool>("obscure"));
            service->SetDhtBootstrap(item.second.get<std::string>("rendezvous.dht.bootstrap", ""));
            service->SetDhtNetwork(item.second.get<uint32_t>("rendezvous.dht.network", 0));
            data.services[wxUIntPtr(service.get())] = service;
        }
    }
}
