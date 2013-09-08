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
#include "UsernameBox.h"


UsernameBox::UsernameBox(wxWindow *parent, const char* name) : DStripMenuButton(parent, name)
{
	setOffset(10);

	wxSize size = GetBestSize();
	size.x+=getOffset();
	CacheBestSize(size);

	if (size.x < 82)
		size.x = 82;

	SetSize(wxSize( -1,38 ));
	SetMinSize( wxSize( size.x,38 ) );

	SetCursor(wxCURSOR_HAND);


}

UsernameBox::~UsernameBox()
{
}

void UsernameBox::onActiveToggle(bool state)
{
	setActive(state);
}

