/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Ian T. Jacobsen <iantj92@gmail.com>

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


#ifndef DESURA_THEME_H
#define DESURA_THEME_H

#define COLOR_ONE 1
#define COLOR_TWO 2

#define COLOR_BG COLOR_ONE
#define COLOR_FG COLOR_TWO

#include "Common.h"
#include "Color.h"
#include "BaseManager.h"
#include "managers/Managers.h"

class SpriteRect : public BaseItem, public SpriteRectI
{
public:
	SpriteRect(const char* name) : BaseItem(name)
	{
	}

	virtual int32 getX()
	{
		return x;
	}

	virtual int32 getY()
	{
		return y;
	}

	virtual int32 getW()
	{
		return w;
	}

	virtual int32 getH()
	{
		return h;
	}


	int32 x;
	int32 y;
	int32 w;
	int32 h;
};






class ThemeColorInfo : public BaseItem
{
public:
	ThemeColorInfo(const char* name) : BaseItem(name)
	{
	}

	Color color;
};

class ThemeImageInfo : public BaseItem
{
public:
	ThemeImageInfo(const char* name) : BaseItem(name)
	{
	}

	gcString path;
};

class ThemeSpriteInfo : public BaseItem, public BaseManager<SpriteRect>
{
public:
	ThemeSpriteInfo(const char* name) : BaseItem(name), BaseManager<SpriteRect>(true)
	{
	}

	void addItem(SpriteRect* sprite)
	{
		BaseManager<SpriteRect>::addItem(sprite);
	}
};

class ThemeWebInfo : public BaseItem
{
public:
	ThemeWebInfo(const char* name) : BaseItem(name)
	{
	}

	gcString path;
};

typedef BaseManager<ThemeColorInfo> ColorList;

class ThemeControlInfo : public BaseItem, public ColorList
{
public:
	ThemeControlInfo(const char* name) : BaseItem(name), ColorList(true)
	{
	}

	Color getColor(const char* control, const char* id)
	{
		ThemeColorInfo* col = ColorList::findItem(id);

		if (col)
		{
			return col->color;
		}
		else
		{
			Warning(gcString("Cant find control {0} color {1}\n", control, id));
			return Color();
		}
	}

	void add(ThemeColorInfo* col)
	{
		ColorList::addItem(col);
	}
};

typedef BaseManager<ThemeWebInfo> WebList;
typedef BaseManager<ThemeControlInfo> ControlList;
typedef BaseManager<ThemeImageInfo> ImageList;
typedef BaseManager<ThemeSpriteInfo> SpriteList;


//! Repersents a theme file
//!
class Theme : private ControlList, private ImageList, private SpriteList, private WebList
{
public:
	Theme(const char* name);

	//! Gets the path to an image
	//! 
	//! @param id Image name
	//! @return Path to image if exist or null if not
	//!
	const char* getImage(const char* id);

	//! Gets the path to a internal webpagte
	//! 
	//! @param id Web page name
	//! @return Path to web page if exist or null if not
	//!
	const char* getWebPage(const char* id);

	//! Gets a color from the theme
	//!
	//! @param id Color name
	//! @param type Which color to get
	//! @return Color if exist or default Color
	//!
	Color getColor(const char* name, const char* id);

	//! Gets a rectangle from a sprite section
	//!
	//! @param id Sprite name
	//! @param rectId Rectangle name
	//! @return Rectangle info
	//!
	SpriteRect* getSpriteRect(const char* id, const char* rectId);

	//! Gets the hash of this themes name
	//!
	//! @return Theme name hash
	//!
	uint32 getHash(){return m_uiHash;}

	//! Parses a theme xml file
	//!
	//! @param file Location of xml
	//!
	void parseFile(const char* file);


private:
	uint32 m_uiHash;

	gcString m_szName;
	gcString m_szPName;
	gcString m_szDev;
};




#endif
