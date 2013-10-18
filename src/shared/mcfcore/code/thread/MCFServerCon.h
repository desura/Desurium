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

#ifndef DESURA_MCFServerConNECTION_H
#define DESURA_MCFServerConNECTION_H
#ifdef _WIN32
#pragma once
#endif

#include "util_thread/BaseThread.h"
#include "mcfcore/MCFI.h"

namespace MCFCore
{
namespace Misc
{

class OutBufferI
{
public:
	virtual bool writeData(char* data, uint32 size)=0;
	virtual void reset()=0;
};

class MCFServerConI
{
public:
	virtual ~MCFServerConI(){}

	//! Connect to mcf server
	//!
	//! @param host Sever to connect to in form of name:port
	//! @param fileAuth File authentication cookie
	//!
	virtual void connect(const char* host, MCFCore::Misc::GetFile_s* fileAuth) = 0;

	//! Disconnect from server. Gets called in the destructor automatically
	//!
	virtual void disconnect() = 0;

	//! Set download provider name
	//!
	virtual void setDPInformation(const char* name) = 0;

	//! Downloads a range from the selected file
	//!
	//! @param offset Offset into the selected mcf
	//! @param size Size of the range to download
	//! @param buff Static buffer to store result into. If null it will create its own buffer
	//!
	virtual void downloadRange(uint64 offset, uint32 size, OutBufferI* buff) = 0;


	//! Download progress event
	//!
	Event<uint32> onProgressEvent;

	//! Pause the download. This will cause downloadRange to return early
	//!
	virtual void onPause() = 0;

	//! Is the client connected to the server
	//!
	virtual bool isConnected() = 0;
};

//! MCFServerCon is used to download mcf content from mcf servers
//!
class MCFServerCon : public MCFServerConI
{
public:
	MCFServerCon();
	~MCFServerCon();

	void connect(const char* host, MCFCore::Misc::GetFile_s* fileAuth) override;
	void disconnect() override;

	void setDPInformation(const char* name) override;
	void downloadRange(uint64 offset, uint32 size, OutBufferI* buff) override;

	void onPause() override;

	bool isConnected() override
	{
		return m_bConnected;
	}

protected:
	void doDownloadRange(uint64 offset, uint32 size);
	
	void onProgress(Prog_s &prog);
	void onWrite(WriteMem_s &mem);

private:
	HttpHandle m_FtpHandle;

	uint32 m_uiDPRId;
	bool m_bConnected;

	uint64 m_uiDone;

	OutBufferI* m_pOutBuffer;
};


}
}

#endif //DESURA_MCFServerConNECTION_H
