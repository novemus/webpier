#include <ui/messagedialog.h>
#include <ui/logo.h>

CMessageDialog::CMessageDialog( wxWindow* parent, const wxString& message, long style, wxWindowID id, const wxPoint& pos, const wxSize& size ) 
    : wxDialog( parent, id, _("WebPier"), pos, size, style )
{
	this->SetIcon(::GetAppIconBundle().GetIcon());
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* mainSizer;
	mainSizer = new wxBoxSizer( wxVERTICAL );

    auto kind = style & wxICON_ERROR ? wxART_ERROR : style & wxICON_WARNING ? wxART_WARNING : style & wxICON_QUESTION ? wxART_QUESTION : wxART_INFORMATION;
	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxArtProvider::GetBitmap( wxASCII_STR(kind), wxASCII_STR(wxART_MESSAGE_BOX) ), wxDefaultPosition, wxDefaultSize, 0 );
	mainSizer->Add( m_bitmap, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_text = new wxStaticText( this, wxID_ANY, message, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL );
	m_text->Wrap( 500 );
	mainSizer->Add( m_text, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 10 );

	wxStdDialogButtonSizer* sdbSizer;
	sdbSizer = new wxStdDialogButtonSizer();
	m_ok = new wxButton( this, style & wxICON_QUESTION ? wxID_YES : wxID_OK );
	sdbSizer->AddButton( m_ok );

    if (style & wxICON_QUESTION)
    {
        m_no = new wxButton( this, wxID_NO );
	    sdbSizer->AddButton( m_no );
    }

	sdbSizer->Realize();

	mainSizer->Add( sdbSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 10 );

	this->SetSizer( mainSizer );
	this->Layout();
	mainSizer->Fit( this );

	this->Centre( wxBOTH );

    m_ok->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CMessageDialog::onOkButtonClick ), NULL, this );
    if (m_no)
	    m_no->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CMessageDialog::onNoButtonClick ), NULL, this );
}

CMessageDialog::~CMessageDialog()
{
    delete m_bitmap;
    delete m_text;
    delete m_ok;
    delete m_no;
}
