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
#include "gcWebFakeBrowser.h"
#include "wx_controls/gcPanel.h"
#include "Managers.h"

gcWebFakeBrowser::gcWebFakeBrowser(gcPanel* parent)
{
	m_pParent = parent;
	m_szNoLoadString = Managers::GetString(L"#BH_NO_LOAD");
}

void gcWebFakeBrowser::destroy()
{
	delete this;
}

#ifdef WIN32
void gcWebFakeBrowser::onPaint()
{
	wxPaintDC dc(m_pParent);

	wxColor bg = m_pParent->GetBackgroundColour();
	wxColor fg = m_pParent->GetForegroundColour();

	dc.SetFont(m_pParent->GetFont());
	dc.SetTextForeground(*wxWHITE);
	dc.SetTextBackground(*wxBLACK);

	wxSize s = m_pParent->GetSize();
	
	dc.SetBackground(wxBrush(*wxBLACK));
	dc.Clear();

	wxSize te = dc.GetTextExtent(m_szNoLoadString);

	int x = (s.GetWidth()-te.GetWidth())/2;
	int y = (s.GetHeight()-te.GetHeight())/2;

	dc.SetBrush(wxBrush(*wxBLACK));
	dc.DrawText(m_szNoLoadString, x, y);
}
#endif