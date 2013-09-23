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

#ifndef DESURA_UMCF_EX_H
#define DESURA_UMCF_EX_H
#ifdef _WIN32
#pragma once
#endif

#include "umcf/UMcf.h"

class UMcfEx : public UMcf
{
public:
	UMcfEx();
	~UMcfEx();

	//Downloads update info from web
	void getUpdateInfo(bool save = false);

	//downloads the MCF ready to install.
	void downloadMcf();
	bool checkMcf();
	void deleteMcf();

	const char* getLastError(){return m_szLastError;}

	void setProgLevel(uint8 level){m_uiLevel = level;}

protected:
	uint32 progressUpdate(Prog_s *info, uint32 other=0);

private:
	uint8 m_uiLevel;
	char* m_szLastError;
};

#endif //DESURA_UMCF_EX_H
