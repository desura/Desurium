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


#include "Common.h"
#include "MCFServerCon.h"
#include "MCFDPReporter.h"

#define READ_SIZE (1024*1024)

namespace MCFCore
{
namespace Misc
{

MCFServerCon::MCFServerCon()
{
	m_uiDPRId = -1;
	m_bConnected = false;

	m_FtpHandle->getProgressEvent() += delegate(this, &MCFServerCon::onProgress);
	m_FtpHandle->getWriteEvent() += delegate(this, &MCFServerCon::onWrite);

	m_pOutBuffer = NULL;
}

MCFServerCon::~MCFServerCon()
{
	disconnect();

	if (m_uiDPRId != UINT_MAX)
		GetDPReporter()->delProvider(m_uiDPRId);
}

void MCFServerCon::onProgress(Prog_s &prog)
{
	if (!m_bConnected)
		prog.abort = true;
}

void MCFServerCon::onWrite(WriteMem_s &mem)
{
	if (!m_bConnected)
		mem.stop = true;

	if (m_pOutBuffer)
	{
		uint32 ammount = mem.size;
		onProgressEvent(ammount);

		if (m_uiDPRId != UINT_MAX)
			GetDPReporter()->reportProgress(m_uiDPRId, ammount);

		mem.wrote = mem.size;
		mem.handled = true;

		bool res = m_pOutBuffer->writeData((char*)mem.data, mem.size);

		if (!res)
			mem.stop = true;
	}
}

void MCFServerCon::connect(const char* url, GetFile_s* pFileAuth)
{
	if (m_bConnected)
		return;

	gcString u(url);

	if (u.find("mcf://") == 0)
		u.replace(0, 3, "ftp");

	size_t pos = u.find(":62001");

	if (pos != std::string::npos)
		u.replace(pos, 6, ":62003");

	u += "/mcf";

	m_FtpHandle->setUrl(u.c_str());
	m_FtpHandle->setUserPass(pFileAuth->authkey, pFileAuth->authhash);

	m_bConnected = true;
}

void MCFServerCon::disconnect()
{
	if (!m_bConnected)
		return;

	m_bConnected = false;
}

void MCFServerCon::downloadRange(uint64 offset, uint32 size, OutBufferI* buff)
{
	m_pOutBuffer = buff;
	m_uiDone=0;

	try
	{
		doDownloadRange(offset, size);
	}
	catch (gcException &e)
	{
		m_pOutBuffer = NULL;

		if (e.getSecErrId() == 18) //CURLE_PARTIAL_FILE
			return;
			
		disconnect();
		throw;
	}

	m_pOutBuffer = NULL;
}

void MCFServerCon::doDownloadRange(uint64 offset, uint32 size)
{
	if (!m_bConnected)
		throw gcException(ERR_SOCKET, "Socket not connected");

	m_FtpHandle->cleanUp(false);
	m_FtpHandle->setDownloadRange(offset, size);

	uint8 res = m_FtpHandle->getFtp();

	if (res == UWEB_USER_ABORT)
		throw gcException(ERR_MCFSERVER, ERR_USERCANCELED, "Client canceled data write");
}

void MCFServerCon::setDPInformation(const char* name)
{
	if (m_uiDPRId != UINT_MAX)
		GetDPReporter()->delProvider(m_uiDPRId);

	m_uiDPRId = -1;
	m_uiDPRId = GetDPReporter()->newProvider(name);
}

void MCFServerCon::onPause()
{
	m_FtpHandle->abortTransfer();

	if (m_uiDPRId != UINT_MAX)
		GetDPReporter()->delProvider(m_uiDPRId);

	m_uiDPRId = -1;	
}

}
}
