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
#include "gcThemeManager.h"


gcThemeManager::gcThemeManager() : BaseManager<gcImageInfo>(true)
{
}

gcThemeManager::~gcThemeManager()
{
}

gcImageHandle gcThemeManager::getImageHandle(const char* path)
{
	m_WaitMutex.lock();

	gcString name(path);
	gcImageInfo* h = findItem(name.c_str());

	if (!h)
	{
		//if path name starts with an # that means we need to load from theme
		if (path && path[0] == '#')
		{
			h = new gcImageInfo(getImage(path+1), name.c_str());
		}
		else
		{
			h = new gcImageInfo(path, name.c_str());
		}

		addItem(h);
	}

	gcImageHandle imgHandle(h->getImg(), h->getHash(), this);
	h->incRef();

	m_WaitMutex.unlock();

	return imgHandle;
}

void gcThemeManager::newImgHandle(uint32 hash)
{
	m_WaitMutex.lock();

	gcImageInfo* h = findItem(hash);

	if (h)
		h->incRef();

	m_WaitMutex.unlock();
}

void gcThemeManager::desposeImgHandle(uint32 hash)
{
	m_WaitMutex.lock();

	gcImageInfo* h = findItem(hash);

	if (h)
	{
		h->defRef();
		
		if (h->isEmpty())
		{
			removeItem(h->getName());
			safe_delete(h);
		}
	}

	m_WaitMutex.unlock();
}

wxBitmap gcThemeManager::getSprite(wxImage& img, const char* spriteId, const char* spriteName)
{
	SpriteRectI* rect = getSpriteRect(spriteId, spriteName);

	if (!rect || !img.IsOk())
		return wxBitmap();

	int w = rect->getW();
	int h = rect->getH();
	int x = rect->getX();
	int y = rect->getY();

	if (w < 0)
		w = img.GetWidth();

	if (h < 0)
		h =	img.GetHeight();

	if (w > img.GetWidth())
		w = img.GetWidth();

	if (h > img.GetHeight())
		h =	img.GetHeight();

	if (x < 0)
		x = 0;

	if (x > img.GetWidth() - w)
		x = 0;

	if (y < 0)
		y = 0;

	if (y > img.GetHeight() - h)
		y = 0;

	return wxBitmap( img.GetSubImage( wxRect(x,y,w,h) ) );
}


const char* gcThemeManager::getWebPage(const char* id)
{
	return GetThemeManager().getWebPage(id);
}

const char* gcThemeManager::getImage(const char* id)
{
	return GetThemeManager().getImage(id);
}

Color gcThemeManager::getColor(const char* name, const char* id)
{
	return GetThemeManager().getColor(name, id);
}

SpriteRectI* gcThemeManager::getSpriteRect(const char* id, const char* rectId)
{
	return GetThemeManager().getSpriteRect(id, rectId);
}

void gcThemeManager::loadFromFolder(const char* folder)
{
	GetThemeManager().loadFromFolder(folder);
}

void gcThemeManager::loadTheme(const char* name)
{
	GetThemeManager().loadTheme(name);
}

void gcThemeManager::getThemeStubList(std::vector<ThemeStubI*> &vList)
{
	GetThemeManager().getThemeStubList(vList);
}

const char* gcThemeManager::getThemeFolder()
{
	return GetThemeManager().getThemeFolder();
}
