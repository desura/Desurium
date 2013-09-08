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

#ifndef DESURA_CVAR_MANAGER_H
#define DESURA_CVAR_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "managers/CVar.h"
#include "BaseManager.h"
#include "sqlite3x.hpp"

#include "managers/Managers.h"

enum
{
	CM_OK =0,
	CM_ERR_NOSETT_NODE,
	CM_ERR_NOUSER_NODE,
	CM_ERR_USER_NOTFOUND,
	CM_ERR_NULLUSER,
};

class CVarManager;

extern CVarManager* g_pCVarMang;

void InitCVarManger();
void DestroyCVarManager();
void SaveCVars();

class CVarManager : public BaseManager<CVar>, public CVarManagerI
{
public:
	CVarManager();
	~CVarManager();

	bool RegCVar(CVar* var);
	void UnRegCVar(CVar* var);

	void cleanUserCvars();

	void loadUser(uint32 userId);
	void loadWinUser();
	void loadNormal();

	void saveAll();
	void saveUser();
	void saveWinUser();
	void saveNormal();

	void loadFromFile(const char* file);

	virtual CVar* findCVar(const char* name);
	virtual void getCVarList(std::vector<CVar*> &vList);

protected:
	void parseXML(tinyxml2::XMLNode *cNode);

	void loadFromDb(sqlite3x::sqlite3_reader &reader);
	void saveToDb(sqlite3x::sqlite3_command &cmd, uint8 flags);

	void loadUser(CVar* var);
	void loadWinUser(CVar* var);
	void loadNormal(CVar* var);

	void loadFromDb(sqlite3x::sqlite3_reader &reader, CVar* var);

	std::wstring getWinUser();

private:
	uint32 m_uiUserId;
	gcString m_szCVarDb;

	bool m_bUserLoaded;
	bool m_bWinUserLoaded;
	bool m_bNormalLoaded;
};

#endif //DESURA_CVAR_MANAGER_H
