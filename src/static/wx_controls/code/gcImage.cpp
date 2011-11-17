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
#include "gcImage.h"

gcImage::gcImage(const char* image, bool alpha) :  wxImage(1,1)
{
	setImage(image, alpha);
}

void gcImage::setImage(const char* image, bool alpha)
{
	if (!image || !UTIL::FS::isValidFile(UTIL::FS::Path(image, "", true)))
	{
		Warning(gcString("Failed to find image [{0}]\n", image));
		return;
	}

	gcWString str = gcWString(image);
	LoadFile(str.c_str(), wxBITMAP_TYPE_ANY);

	if (!IsOk())
	{
		Warning(gcString("Failed to load image {0}\n", image));
	}
	else
	{
		if (alpha)
		{
			ConvertAlphaToMask();
		}
	}
}

void gcImage::tileImg( wxBitmap &target, wxBitmap src, wxColor* fillBg, TILE_DIR dir)
{
	if (!src.IsOk())
		return;

	if ((dir == HOZ || dir == BOTH) && src.GetWidth() == 0)
		return;

	if ((dir == VERT || dir == BOTH) && src.GetHeight() == 0)
		return;

	wxMemoryDC memTargDC(target);

	if (fillBg)
	{
		memTargDC.SetBrush(wxBrush(*fillBg));
		memTargDC.SetPen( wxPen(*fillBg,1) );
		memTargDC.DrawRectangle(0, 0, target.GetWidth(), target.GetHeight());
	}

	if (dir == HOZ)
	{
		for (int32 x=0; x<target.GetWidth(); x+=src.GetWidth())
			memTargDC.DrawBitmap(src, x, 0, true);
	}
	else if (dir == VERT)
	{
		for (int32 y=0; y<target.GetHeight(); y+=src.GetHeight())
			memTargDC.DrawBitmap(src, 0, y, true);
	}
	else
	{
		for (int32 x=0; x<target.GetWidth(); x+=src.GetWidth())
		{
			for (int32 y=0; y<target.GetHeight(); y+=src.GetHeight())
				memTargDC.DrawBitmap(src, x, y, true);
		}
	}

	memTargDC.SelectObject(wxNullBitmap);
}

