#include "startupdialog.h"
#include "messagedialog.h"
#include "logo.h"
#include <wx/valtext.h>

CStartupDialog::CStartupDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) :
        wxDialog(parent, id, title, pos, size, style)
{
    static constexpr const char* FORBIDDEN_PATH_CHARS = "*/\\<>:|? \t\n\r";

    this->SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer* mainSizer;
    mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBoxSizer* idSizer;
    idSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("")), wxVERTICAL);

    auto image = ::GetLogo();
    wxBitmapHelpers::Rescale(image, wxSize(64, 64));
    m_logo = new wxStaticBitmap(idSizer->GetStaticBox(), wxID_ANY, image, wxDefaultPosition, wxDefaultSize, 0);
    idSizer->Add(m_logo, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

    m_welcome = new wxStaticText(
        idSizer->GetStaticBox(),
        wxID_ANY,
        _("Welcome to the WebPier!"),
        wxDefaultPosition,
        wxDefaultSize,
        0);
    m_welcome->Wrap(-1);
    idSizer->Add(m_welcome, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 20);
    m_message = new wxStaticText(
        idSizer->GetStaticBox(),
        wxID_ANY,
        _("The WebPier program is designed to forward TCP services running on machines located behind the NAT. This\n"
          "is a completely P2P application and no third-party relay services are used to link local and remote machines. The\n"
          "well-known UDP hole-punching technique is used to establish connections, and email or DHT-network is used as\n"
          "a rendezvous service. Public key authentication is used for rendezvous security. The email service is also used to\n"
          "invite owners of other machines to participate and to exchange public keys. Thus, to use the application, you at\n"
          "least need to define the identifier of this pier, setup the built-in email client and define STUN server. Note that\n"
          "the email address is a first part of pier's identifier and the pier name must be unique for the address when it is\n"
          "used for another piers. To init the pier context you should define pier identifier right now."),
        wxDefaultPosition,
        wxDefaultSize,
        0);
    m_message->Wrap(-1);
    idSizer->Add(m_message, 0, wxALL, 20);

    wxFlexGridSizer* idGridSizer;
    idGridSizer = new wxFlexGridSizer(0, 2, 5, 5);
    idGridSizer->AddGrowableCol(1);
    idGridSizer->SetFlexibleDirection(wxBOTH);
    idGridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxStaticText* ownerLabel;
    ownerLabel = new wxStaticText(idSizer->GetStaticBox(), wxID_ANY, _("Owner"), wxDefaultPosition, wxSize(50, -1), 0);
    ownerLabel->Wrap(-1);
    idGridSizer->Add(ownerLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_ownerCtrl = new wxTextCtrl(idSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0);
    m_ownerCtrl->SetToolTip(_("Email address to represent you to owners of remote piers"));
    m_ownerCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator*)m_ownerCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add(m_ownerCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    wxStaticText* pierLabel;
    pierLabel = new wxStaticText(idSizer->GetStaticBox(), wxID_ANY, _("Pier"), wxDefaultPosition, wxSize(50, -1), 0);
    pierLabel->Wrap(-1);
    idGridSizer->Add(pierLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_pierCtrl = new wxTextCtrl(idSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0);
    m_pierCtrl->SetToolTip(_("Identifier of this pier"));
    m_pierCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator*)m_ownerCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add(m_pierCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    idSizer->Add(idGridSizer, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 10);

    mainSizer->Add(idSizer, 1, wxALL | wxEXPAND, 10);

    wxStdDialogButtonSizer* sdbSizer = new wxStdDialogButtonSizer();
    wxButton* sdbSizerOK = new wxButton(this, wxID_OK);
    sdbSizer->AddButton(sdbSizerOK);
    sdbSizer->Realize();

    mainSizer->Add(sdbSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxRIGHT | wxLEFT, 10);

    this->SetSizer(mainSizer);
    this->Layout();
    mainSizer->Fit(this);

    this->Centre(wxBOTH);

    sdbSizerOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CStartupDialog::onOkButtonClick), NULL, this);
    this->Bind(wxEVT_CLOSE_WINDOW, &CStartupDialog::onCloseButtonClick, this);
}

void CStartupDialog::onCloseButtonClick(wxCloseEvent& event)
{
    this->EndModal(wxID_CANCEL);
    event.Skip();
}

void CStartupDialog::onOkButtonClick(wxCommandEvent& event)
{
    if (m_ownerCtrl->GetValue().IsEmpty() || m_pierCtrl->GetValue().IsEmpty())
    {
        CMessageDialog dialog(this, _("You must define Owner and Pier parameters"), wxDEFAULT_DIALOG_STYLE | wxICON_ERROR);
        dialog.ShowModal();
    }
    else
        event.Skip();
}

CStartupDialog::~CStartupDialog()
{
    delete m_logo;
    delete m_welcome;
    delete m_message;
    delete m_ownerCtrl;
    delete m_pierCtrl;
}
