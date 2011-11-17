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

#ifndef DESURA_URLMANAGER_H
#define DESURA_URLMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "mcfcore/DownloadProvider.h"
#include "mcfcore/MCFI.h"

#include "util_thread/BaseThread.h"
#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;


namespace MCFCore
{
namespace Misc
{

class ErrorInfo;

//! Provider manager handles all the providers for mcf download including error managment
//!
class ProviderManager
{
public:
	//! Constuctor
	//!
	//! @param source Provider source lise
	//!
	ProviderManager(std::vector<MCFCore::Misc::DownloadProvider*> &source);
	~ProviderManager();

	//! Gets new url for download
	//!
	//! @param id Agent id
	//! @param errCode Reason why requesting new url
	//! @param errMsg Detail error message
	//! @return url if has valid url or "NULL" if no valid url
	//!
	gcString requestNewUrl(uint32 id, uint32 errCode, const char* errMsg);

	//! Gets a url for agent
	//!
	//! @param id Agent id
	//! @return url if has valid url or "NULL" if no valid url
	//!
	gcString getUrl(uint32 id);

	//! Gets a name for agent
	//!
	//! @param id Agent id
	//! @return name
	//!
	gcString getName(uint32 id);

	//! Removes an Agent from downloading
	//!
	//! @param id Agent id
	//! @param setTimeOut Trigger timeout for that Agents current url
	//!
	void removeAgent(uint32 id, bool setTimeOut = false);

	//! Returns if any workers have valid urls at this stage
	//!
	//! @return True if valid urls, false if not
	//!
	bool hasValidAgents();

	//! Gets the providers list
	//!
	//! @return Provider list
	//!
	std::vector<MCFCore::Misc::DownloadProvider*>& getVector(){return m_vSourceList;}

	//! Event that gets triggered when using new providers
	//!
	Event<MCFCore::Misc::DP_s> onProviderEvent;

private:

	std::vector<MCFCore::Misc::DownloadProvider*> &m_vSourceList;
	std::vector<ErrorInfo*> m_vErrorList;
	::Thread::Mutex m_WaitMutex;
};

}
}

#endif //DESURA_URLMANAGER_H
