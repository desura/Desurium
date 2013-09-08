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


#ifndef DESURA_THEME_MANAGER_H
#define DESURA_THEME_MANAGER_H

#include "managers/Managers.h"
#include "Theme.h"


class ThemeManager : public ThemeManagerI
{
public:
	ThemeManager();
	~ThemeManager();

	virtual const char* getThemeFolder();
	virtual const char* getWebPage(const char* id);
	virtual const char* getImage(const char* id);
	virtual Color getColor(const char* name, const char* id);

	//! Gets a rectangle from a sprite section
	//!
	//! @param id Sprite name
	//! @param rectId Rectangle name
	//! @return Rectangle info
	//!
	virtual SpriteRectI* getSpriteRect(const char* id, const char* rectId);

	virtual void loadFromFolder(const char* folder);
	virtual void loadTheme(const char* theme);

	virtual void getThemeStubList(std::vector<ThemeStubI*> &vList);

private:
	Theme *m_pTheme;

	gcString m_szPath;
	gcString m_szThemeFolder;
};

#endif
