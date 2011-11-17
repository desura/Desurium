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



#ifndef LANG_MANAGER_H
#define LANG_MANAGER_H

#include "BaseManager.h"
#include "managers/Managers.h"

class LanguageString : public BaseItem
{
public:
	LanguageString(const char* name) : BaseItem(name)
	{
	}

	gcString str;
	gcWString ustr;
};


enum
{
	LM_OK=0,
	LM_ERR_NOLANG_NODE,
};

class LanguageManager : public BaseManager<LanguageString>, public LanguageManagerI
{
public:
	LanguageManager(const char* defaultLangFile);
	~LanguageManager();

	virtual const char* getString(const char* name);
	virtual const wchar_t* getString(const wchar_t* name);

	virtual bool loadFromFile(const char* file);
};











#endif
