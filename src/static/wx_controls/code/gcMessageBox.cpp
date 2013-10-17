/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "Common.h"
#include "gcMessageBox.h"
#include <wx/clipbrd.h>

void gcErrorBox(wxWindow *parent, const char* title, const char* prompt, const gcException &e, HelperButtonsI* helper)
{
	gcWString t(title);
	gcWString p(prompt);

	if (title && title[0] == '#')
	{
		t = gcWString(Managers::GetString(title));
	}

	if (prompt && prompt[0] == '#')
	{
		p = gcWString(Managers::GetString(prompt));
	}

	gcWString errMsg(L"{0}\n\n{1}", p, e);
	Warning(gcWString(L"{0}: {1}\n", p, e));

	gcMessageBox(parent, errMsg, t, wxICON_EXCLAMATION|wxOK, helper);
}

int gcMessageBox(wxWindow *parent, const wxString& message, const wxString& caption, long style, HelperButtonsI* helper)
{
	long decorated_style = style;

	if (!HasAnyFlags(style, (wxICON_EXCLAMATION|wxICON_HAND|wxICON_INFORMATION|wxICON_QUESTION)))
	{
		decorated_style |= ( style & wxYES ) ? wxICON_QUESTION : wxICON_INFORMATION ;
	}

	gcMessageDialog dialog(parent, message, caption, decorated_style);
	dialog.addHelper(helper);

	int ans = dialog.ShowModal();
	
	if (ans == wxID_OK)
		return wxOK;
	else if (ans == wxID_YES)
		return wxYES;
	else if (ans == wxID_NO)
		return wxNO;

	return wxCANCEL;
}


gcMessageDialog::gcMessageDialog(wxWindow* parent, const wxString& message, const wxString& caption, long style, const wxPoint& pos) 
	: gcDialog(NULL, wxID_ANY, caption, pos, wxDefaultSize, wxCAPTION ) //PARENT MUST BE LEFT NULL. Crashes other wise.
{
	if (style&(wxYES))
		m_bButtonList.push_back(new gcButton(this, wxID_YES, Managers::GetString(L"#YES")));

	if (style&(wxOK))
		m_bButtonList.push_back(new gcButton(this, wxID_OK, Managers::GetString(L"#OK")));

	if (style&(wxNO))
		m_bButtonList.push_back(new gcButton(this, wxID_NO, Managers::GetString(L"#NO")));

	if (style&(wxCANCEL))
		m_bButtonList.push_back(new gcButton(this, wxID_CANCEL, Managers::GetString(L"#CANCEL")));

	if (style&(wxAPPLY))
		m_bButtonList.push_back(new gcButton(this, wxID_OK, Managers::GetString(L"#APPLY")));

	if (style&(wxCLOSE))
		m_bButtonList.push_back(new gcButton(this, wxID_OK, Managers::GetString(L"#CLOSE")));	


	m_imgIcon = new gcImageControl( this, wxID_ANY, wxDefaultPosition, wxSize( 48,48 ) );

	int iconmask = (style&wxICON_MASK);

	if (iconmask & wxICON_EXCLAMATION)
	{
		m_imgIcon->setImage(("#icon_warning"));
	}
	else if (iconmask & wxICON_HAND)
	{
		m_imgIcon->setImage(("#icon_hand"));
	}
	else if (iconmask & wxICON_QUESTION)
	{
		m_imgIcon->setImage(("#icon_question"));
	}
	else //if (iconmask & wxICON_INFORMATION)
	{
		m_imgIcon->setImage(("#icon_information"));
	}

	m_labInfo = new wxStaticText( this, wxID_ANY, message, wxDefaultPosition, wxSize( 280 ,-1), 0 );
	m_labInfo->Wrap( 260 );


	wxFlexGridSizer* fgSizer2;
	fgSizer2 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer2->SetFlexibleDirection( wxBOTH );
	fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer2->Add( m_imgIcon, 0, wxALL, 10 );
	fgSizer2->Add( m_labInfo, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	

	m_bsButtonSizer = new wxBoxSizer( wxHORIZONTAL );
	m_bsButtonSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	
	for (size_t x=0; x<m_bButtonList.size(); x++)
		m_bsButtonSizer->Add(m_bButtonList[x], 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	if (m_bButtonList.size() > 0)
	{
		gcButton* last = m_bButtonList[m_bButtonList.size()-1];

		last->SetFocus();
		last->SetDefault();
	}

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 1, 0, 0 );
	fgSizer1->AddGrowableCol( 0 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	fgSizer1->Add( fgSizer2, 1, wxEXPAND, 5 );
	fgSizer1->Add( m_bsButtonSizer, 1, wxEXPAND, 5 );

	this->SetSizer(fgSizer1);
	this->SetSize(getBestTextSize());
	this->Layout();

	centerOnParent(parent);
	
#ifdef NIX
	this->Raise();
#endif

	m_pHelper = NULL;
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &gcMessageDialog::onButtonClick, this);
}

gcMessageDialog::~gcMessageDialog()
{
}

void gcMessageDialog::centerOnParent(wxWindow* p)
{
	if (!p)
		p = GetMainWindow();

	if (!p)
		return;

	wxSize ps = p->GetSize();
	wxPoint pp = p->GetPosition();

	wxSize mid = ps/2+wxSize(pp.x, pp.y);

	wxSize ts = this->GetSize();
	ts = ts/2;

	wxSize fin = mid - ts;

	this->SetPosition(wxPoint(fin.x, fin.y));
}

void gcMessageDialog::addHelper(HelperButtonsI* helper)
{
	if (!helper)
		return;

	m_pHelper = helper;

	m_bsButtonSizer->Clear();
	m_bsButtonSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	for (size_t x=0; x<m_pHelper->getCount(); x++)
	{
		gcButton *button = new gcButton(this, wxID_ANY, gcWString(m_pHelper->getLabel(x)));
		button->SetToolTip(gcWString(m_pHelper->getToolTip(x)));

		m_bsButtonSizer->Add(button, 0, wxTOP|wxBOTTOM|wxRIGHT, 5);
		m_vHelperList.push_back(button);
	}

	for (size_t x=0; x<m_bButtonList.size(); x++)
		m_bsButtonSizer->Add(m_bButtonList[x], 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );
		
	this->Layout();
}

wxSize gcMessageDialog::getBestTextSize()
{
	wxSize txtSize = m_labInfo->GetBestSize();

	int lableSize = txtSize.GetHeight() + 10;
	int iconSize = 48+20;
	int buttonSize = 30 + 10;

	if (lableSize < iconSize)
		lableSize = iconSize;

	txtSize.SetWidth(370);
	txtSize.SetHeight( lableSize + buttonSize + 20);
	
	if (txtSize.GetHeight() < 120)
		txtSize.SetHeight(120);

	return txtSize;
}

void gcMessageDialog::onButtonClick(wxCommandEvent& event)
{
	if (m_pHelper)
	{
		for (size_t x=0; x<m_vHelperList.size(); x++)
		{
			if (m_vHelperList[x]->GetId() == event.GetId())
			{
				m_pHelper->performAction(x);
				break;
			}
		}
	}

	EndModal(event.GetId());
}
