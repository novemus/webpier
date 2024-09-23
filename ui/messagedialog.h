#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

class CMessageDialog : public wxDialog
{
    wxStaticBitmap* m_bitmap;
    wxStaticText* m_text;
    wxButton* m_ok = nullptr;
    wxButton* m_no = nullptr;

	void onNoButtonClick( wxCommandEvent& event ) { this->EndModal(m_ok->GetId()); event.Skip(); }
	void onOkButtonClick( wxCommandEvent& event ) { this->EndModal(m_ok->GetId()); event.Skip(); }

public:

    CMessageDialog( wxWindow* parent, const wxString& message, long style = wxDEFAULT_DIALOG_STYLE|wxICON_WARNING, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );

    ~CMessageDialog();
};
