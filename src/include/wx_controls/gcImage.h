/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifndef DESURA_GCIMAGE_H
#define DESURA_GCIMAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>

class gcImage : public wxImage
{
public:
	gcImage(const char* image);
	void setImage(const char* image);

	enum TILE_DIR
	{
		HOZ,
		VERT,
		BOTH,
	};

	static void tileImg( wxBitmap &target, wxBitmap src, wxColor* fillBg = NULL, TILE_DIR dir = HOZ);
};

#endif //DESURA_GCIMAGE_H
