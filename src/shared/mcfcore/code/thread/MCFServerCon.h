/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

//! MCFServerCon is used to download mcf content from mcf servers
//!
class MCFServerCon
{
public:
	MCFServerCon();
	~MCFServerCon();

	//! Connect to mcf server
	//!
	//! @param host Sever to connect to in form of name:port
	//! @param fileAuth File authentication cookie
	//!
	void connect(const char* host, MCFCore::Misc::GetFile_s* fileAuth);

	//! Disconnect from server. Gets called in the deconstuctor automattically
	//!
	void disconnect();

	//! Set download provider name
	//!
	void setDPInformation(const char* name);

	//! Downloads a range from the selected file
	//!
	//! @param offset Offset into the selected mcf
	//! @param size Size of the range to download
	//! @param buff Static buffer to store result into. If null it will create its own buffer
	//!
	void downloadRange(uint64 offset, uint32 size, OutBufferI* buff);


	//! Download progress event
	//!
	Event<uint32> onProgressEvent;

	//! Pause the download. This will cause downloadRange to return early
	//!
	void onPause();

	//! Is the client connected to the server
	//!
	bool isConnected(){return m_bConnected;}

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
