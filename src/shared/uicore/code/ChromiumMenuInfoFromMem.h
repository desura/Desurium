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

#ifndef DESURA_CHROMIUMMENUINFOFROMMEM_H
#define DESURA_CHROMIUMMENUINFOFROMMEM_H
#ifdef _WIN32
#pragma once
#endif

#include "cef_desura_includes/ChromiumBrowserI.h"

class MenuItem;

class ChromiumMenuInfoFromMem : public ChromiumDLL::ChromiumMenuInfoI
{
public:
	ChromiumMenuInfoFromMem(const char* buff, uint32 size);
	~ChromiumMenuInfoFromMem();

	virtual TypeFlags getTypeFlags();
	virtual EditFlags getEditFlags();

	virtual void getMousePos(int* x, int* y);

	virtual const char* getLinkUrl();
	virtual const char* getImageUrl();
	virtual const char* getPageUrl();
	virtual const char* getFrameUrl();
	virtual const char* getSelectionText();
	virtual const char* getMisSpelledWord();
	virtual const char* getSecurityInfo();

	virtual int getCustomCount();
	virtual ChromiumDLL::ChromiumMenuItemI* getCustomItem(size_t index);

	virtual int* getHWND();

private:
	TypeFlags m_Type;
	EditFlags m_Edit;

	int m_X;
	int m_Y;

	gcString m_szLinkUrl;
	gcString m_szImgUrl;
	gcString m_szPageUrl;
	gcString m_szFrameUrl;
	gcString m_szSelectionText;
	gcString m_szMisSpelledWord;
	gcString m_szSecurityInfo;

	int* m_Hwnd;

	std::vector<MenuItem*> m_vMenuItems;
};

#endif //DESURA_CHROMIUMMENUINFOFROMMEM_H
