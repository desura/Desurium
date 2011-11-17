/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

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
#include "ButtonStrip.h"

#include "MainApp.h"


BEGIN_EVENT_TABLE( ButtonStrip, gcPanel )
	EVT_BUTTON( wxID_ANY, ButtonStrip::onButtonClicked )
END_EVENT_TABLE()

ButtonStrip::ButtonStrip(wxWindow *parent) : gcPanel(parent)
{
	m_imgBg = GetGCThemeManager()->getImageHandle("#header_bg");
	//SetBackgroundColour( wxColour( 255, 128, 0 ) );
	//SetSize(wxSize( 90,30 ));

	m_pThreadButton = new gcImgButtonCount(this, wxID_ANY, wxDefaultPosition, wxSize(27,27));
	m_pThreadButton->setDefaultImage(("#header_button_plus"));
	m_pThreadButton->setHoverImage(("#header_button_plus_hover"));
	m_pThreadButton->SetToolTip(Managers::GetString(L"#BS_PLUS"));

	m_pCartButton = new gcImgButtonCount(this, wxID_ANY, wxDefaultPosition, wxSize(27,27));
	m_pCartButton->setDefaultImage(("#header_button_cart"));
	m_pCartButton->setHoverImage(("#header_button_cart_hover"));
	m_pCartButton->SetToolTip(Managers::GetString(L"#BS_CART"));

	m_pPMsgButton = new gcImgButtonCount(this, wxID_ANY, wxDefaultPosition, wxSize(27,27));
	m_pPMsgButton->setDefaultImage(("#header_button_mail"));
	m_pPMsgButton->setHoverImage(("#header_button_mail_hover"));
	m_pPMsgButton->SetToolTip(Managers::GetString(L"#BS_PMESSAGE"));

	m_pUpdateButton = new gcImgButtonCount(this, wxID_ANY, wxDefaultPosition, wxSize(27,27));
	m_pUpdateButton->setDefaultImage(("#header_button_message"));
	m_pUpdateButton->setHoverImage(("#header_button_message_hover"));
	m_pUpdateButton->SetToolTip(Managers::GetString(L"#BS_UPDATE"));



	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 1, 4, 0, 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	fgSizer1->Add( m_pPMsgButton, 0, 0, 5 );
	fgSizer1->Add( m_pCartButton, 0, 0, 5 );
	fgSizer1->Add( m_pUpdateButton, 0, 0, 5 );
	fgSizer1->Add( m_pThreadButton, 0, 0, 5 );
	
	this->SetSizer( fgSizer1 );
	this->Layout();

	Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( ButtonStrip::OnEraseBG ) );
	*GetUserCore()->getUserUpdateEvent() += delegate(this, &ButtonStrip::onUserUpdate);

	m_pPMsgButton->setCount(GetUserCore()->getPmCount());
	m_pUpdateButton->setCount(GetUserCore()->getUpCount());
}

ButtonStrip::~ButtonStrip()
{
	if (GetUserCore())
		*GetUserCore()->getUserUpdateEvent() -= delegate(this, &ButtonStrip::onUserUpdate);
}


void ButtonStrip::onButtonClicked(wxCommandEvent& event)
{
	if (event.GetId() == m_pThreadButton->GetId())
	{	
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::ThreadWatch).c_str(), COMMUNITY);
	}
	else if (event.GetId() == m_pPMsgButton->GetId())
	{	
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::Inbox).c_str(), COMMUNITY);
	}
	else if (event.GetId() == m_pUpdateButton->GetId())
	{	
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::Updates).c_str(), COMMUNITY);
	}
	else if (event.GetId() == m_pCartButton->GetId())
	{	
		g_pMainApp->loadUrl(GetWebCore()->getUrl(WebCore::Cart).c_str(), COMMUNITY);
	}
}

void ButtonStrip::OnEraseBG( wxEraseEvent& event )
{
	if (!m_imgBg.getImg() || !m_imgBg->IsOk())
	{
		event.Skip();
	}
	else
	{
		wxDC *dc = event.GetDC();
		wxBitmap temp(m_imgBg->Scale(GetSize().GetWidth(), GetSize().GetHeight()));
		dc->DrawBitmap(temp, 0,0, false);
	}
}

void ButtonStrip::onUserUpdate()
{
	m_pPMsgButton->setCount(GetUserCore()->getPmCount());
	m_pUpdateButton->setCount(GetUserCore()->getUpCount());
	m_pCartButton->setCount(GetUserCore()->getCartCount());
	m_pThreadButton->setCount(GetUserCore()->getThreadCount());
}
