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

#ifndef DESURA_GCTHEMELOADER_H
#define DESURA_GCTHEMELOADER_H
#ifdef _WIN32
#pragma once
#endif

#include "gcSchemeBase.h"

class ThemeLoaderScheme : public DesuraSchemeBase<ThemeLoaderScheme>
{
public:
	ThemeLoaderScheme();

	virtual bool processRequest(ChromiumDLL::SchemeRequestI* request, bool* redirect);
	virtual void cancel();
	virtual bool read(char* buffer, int size, int* readSize);

private:
	gcString m_szData;
};


class StaticLoaderScheme : public DesuraSchemeBase<StaticLoaderScheme>
{
public:
	StaticLoaderScheme();

	virtual bool processRequest(ChromiumDLL::SchemeRequestI* request, bool* redirect);
	virtual void cancel();
	virtual bool read(char* buffer, int size, int* readSize);

private:
	gcString m_szData;
};

#endif //DESURA_GCTHEMELOADER_H
