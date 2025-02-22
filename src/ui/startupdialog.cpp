#include <ui/startupdialog.h>
#include <ui/messagedialog.h>
#include <ui/logo.h>
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

    m_logo = new wxStaticBitmap(idSizer->GetStaticBox(), wxID_ANY, ::GetAppIconBundle().GetIconOfExactSize(wxSize(128, 128)), wxDefaultPosition, wxDefaultSize, 0);
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
        _("The WebPier program is designed to forward TCP services running on machines located behind the NAT. This is a\n"
          "completely P2P application and no third-party services are used to relay connection between sides. The well-known\n"
          "UDP-hole-punching technique is used to pass through the NAT, and email or DHT network is used as a rendezvous\n"
          "service. Rendezvous security is ensured by the public key authentication. To get started, you at least need to define\n"
          "the identifier of this pier, then set up your preferred rendezvous service and define a STUN server. Note that the\n"
          "email address is a first part of pier's identifier and the pier name must be unique for the address."),
        wxDefaultPosition,
        wxDefaultSize,
        0);
    m_message->Wrap(-1);
    idSizer->Add(m_message, 0, wxALL, 20);

    m_prompt = new wxStaticText(
        idSizer->GetStaticBox(),
        wxID_ANY,
        _("Define the pier identifier to init the WebPier context"),
        wxDefaultPosition,
        wxDefaultSize,
        0);
    m_prompt->Wrap(-1);
    idSizer->Add(m_prompt, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 20);

    wxFlexGridSizer* idGridSizer;
    idGridSizer = new wxFlexGridSizer(0, 2, 5, 5);
    idGridSizer->AddGrowableCol(1);
    idGridSizer->SetFlexibleDirection(wxBOTH);
    idGridSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    wxStaticText* ownerLabel;
    ownerLabel = new wxStaticText(idSizer->GetStaticBox(), wxID_ANY, _("Owner"), wxDefaultPosition, wxSize(50, -1), 0);
    ownerLabel->Wrap(-1);
    idGridSizer->Add(ownerLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    m_ownerCtrl = new wxTextCtrl(idSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0);
    m_ownerCtrl->SetToolTip(_("Email address to represent you to owners of remote piers"));
    m_ownerCtrl->SetValidator(wxTextValidator(wxFILTER_EXCLUDE_CHAR_LIST));
    ((wxTextValidator*)m_ownerCtrl->GetValidator())->SetCharExcludes(FORBIDDEN_PATH_CHARS);

    idGridSizer->Add(m_ownerCtrl, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxRIGHT | wxLEFT, 5);

    wxStaticText* pierLabel;
    pierLabel = new wxStaticText(idSizer->GetStaticBox(), wxID_ANY, _("Pier"), wxDefaultPosition, wxSize(50, -1), 0);
    pierLabel->Wrap(-1);
    idGridSizer->Add(pierLabel, 0, wxALIGN_CENTER_VERTICAL | wxBOTTOM | wxRIGHT | wxLEFT, 5);

    m_pierCtrl = new wxTextCtrl(idSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0);
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
    delete m_prompt;
    delete m_ownerCtrl;
    delete m_pierCtrl;
}
