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

#ifndef DESURA_GCTHEMEMANAGER_H
#define DESURA_GCTHEMEMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "gcManagers.h"
#include "wx_controls/gcImageHandle.h"
#include "wx_controls/gcImage.h"
#include "util_thread/BaseThread.h"
#include "BaseManager.h"



class gcImageInfo : public BaseItem
{
public: 
	gcImageInfo(const char* path, const char* name, bool alpha = false) : BaseItem(name)
	{
		m_uiRefCount = 0;
		m_pImg = new gcImage(path, alpha);
	}

	~gcImageInfo()
	{
		safe_delete(m_pImg);
	}

	void incRef()
	{
		m_uiRefCount++;
	}

	void defRef()
	{
		m_uiRefCount--;
	}

	bool isEmpty()
	{
		return (m_uiRefCount == 0);
	}

	gcImage* getImg()
	{
		return m_pImg;
	}

private:
	uint32 m_uiRefCount;
	gcImage* m_pImg;
};


class gcThemeManager : public BaseManager<gcImageInfo>, public gcThemeManagerI
{
public:
	gcThemeManager();
	~gcThemeManager();

	//gets image handle. Alpha refers to if the image is the alpha version
	virtual gcImageHandle getImageHandle(const char* path, bool alpha = false);
	virtual wxBitmap getSprite(wxImage& img, const char* spriteId, const char* spriteName);

	virtual const char* getThemeFolder();
	virtual const char* getWebPage(const char* id);
	virtual const char* getImage(const char* id);

	virtual Color getColor(const char* name, const char* id);

	virtual SpriteRectI* getSpriteRect(const char* id, const char* rectId);

	virtual void loadFromFolder(const char* folder);
	virtual void loadTheme(const char* name);

	//make sure to destroy them
	virtual void getThemeStubList(std::vector<ThemeStubI*> &vList);

protected:
	void newImgHandle(uint32 hash);
	void desposeImgHandle(uint32 hash);

	Thread::Mutex m_WaitMutex;
	friend class gcImageHandle;
};


#endif //DESURA_GCTHEMEMANAGER_H
