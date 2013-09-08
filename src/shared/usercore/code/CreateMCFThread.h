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

#ifndef DESURA_MCF_CREATE_H
#define DESURA_MCF_CREATE_H
#ifdef _WIN32
#pragma once
#endif


#include "MCFThread.h"


namespace UserCore
{
namespace Thread
{

class CreateMCFThread : public MCFThread
{
public:
	CreateMCFThread(DesuraId id, const char* path);

	//need this here otherwise stop gets called to late
	~CreateMCFThread()
	{
		stop();
	}

	void pauseThread(bool state = true);

protected:
	void doRun();

	void onPause();
	void onUnpause();
	void onStop();

	void compareBranches(std::vector<UserCore::Item::BranchInfo*> &vBranchList);
	void createMcf();

	void waitForItemInfo();

	void retrieveBranchList(std::vector<UserCore::Item::BranchInfo*> &outList);
	
	void processGames(std::vector<UserCore::Item::BranchInfo*> &outList, tinyxml2::XMLElement* platform);
	void processMods(std::vector<UserCore::Item::BranchInfo*> &outList, tinyxml2::XMLElement* game);
	void processBranches(std::vector<UserCore::Item::BranchInfo*> &outList, tinyxml2::XMLElement* item);



private:
	gcString m_szPath;
	gcString m_szFilePath;

	bool m_bComplete;
	uint32 m_iInternId;
};

}
}

#endif //DESURA_MCF_CREATE_H
