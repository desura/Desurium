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
#include "MainFormLeftBorder.h"

#include "wx_controls/gcManagers.h"

MainFormLeftBorder::MainFormLeftBorder(wxWindow* parent) : wxEventDelegateWrapper<gcFlickerFreePaint<gcPanel>>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0)
{
	m_imgBorder = GetGCThemeManager()->getImageHandle("#main_border");

	this->SetMinSize(wxSize(5,-1));
	this->SetMaxSize(wxSize(5,-1));
}

void MainFormLeftBorder::onActiveToggle(bool state)
{
	m_imgBorder = GetGCThemeManager()->getImageHandle(state?"#main_border":"#main_border_nonactive");
	invalidatePaint();
}

void MainFormLeftBorder::doPaint(wxDC* dc)
{
	dc->SetBackground(wxBrush(wxColor(0,0,0),wxBRUSHSTYLE_SOLID));
	dc->Clear();

	int w = GetSize().GetWidth();
	int h = GetSize().GetHeight();

	int iw = m_imgBorder->GetSize().GetWidth();
	int ih = m_imgBorder->GetSize().GetHeight();

	wxImage borderImg = m_imgBorder->Scale(iw, ih);

	SpriteRectI *imgRect = GetGCThemeManager()->getSpriteRect("main_border", "west");

	wxBitmap borderBMP(GetGCThemeManager()->getSprite(borderImg, "main_border", "west").ConvertToImage().GetSubImage(wxRect(1,0,imgRect->getW()-1,imgRect->getH())));
	wxBitmap borderBMPTarg(w,h);

	wxColor c(255,0,255);
	gcImage::tileImg(borderBMPTarg, borderBMP, &c, gcImage::VERT);
	dc->DrawBitmap(borderBMPTarg, 0, 0, true);
}
