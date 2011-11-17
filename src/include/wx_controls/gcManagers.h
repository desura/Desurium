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

#ifndef DESURA_GCMANAGERS_H
#define DESURA_GCMANAGERS_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#include "wx_controls/gcImageHandle.h"
#include "managers/Managers.h"


class gcThemeManagerI : public ThemeManagerI
{
public:
	//! Gets image handle. Alpha refers to if the image has transperency
	//!
	virtual gcImageHandle getImageHandle(const char* path, bool alpha = false)=0;

	//! Gets a sprite from an image
	//!
	virtual wxBitmap getSprite(wxImage& img, const char* spriteId, const char* spriteName)=0;

protected:
	virtual void newImgHandle(uint32 hash)=0;
	virtual void desposeImgHandle(uint32 hash)=0;

	friend class gcImageHandle;
};


gcThemeManagerI* GetGCThemeManager();


extern wxWindow*		GetMainWindow();
extern void				ExitApp();


#endif //DESURA_GCMANAGERS_H
