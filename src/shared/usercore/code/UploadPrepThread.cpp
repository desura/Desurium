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
#include "UploadPrepThread.h"
#include "UploadInfoThread.h"

namespace UserCore
{
namespace Thread
{

UploadPrepThread::UploadPrepThread(DesuraId id, const char* file) : MCFThread( "UploadPrep Thread", id )
{
	m_szFile = gcString(file);
}

void UploadPrepThread::doRun()
{
	UTIL::FS::Path path = UTIL::FS::PathWithFile(m_szFile);

	if (!UTIL::FS::isValidFile(path))
		throw gcException(ERR_INVALIDFILE);


	McfHandle mcfHandle;;

	bool validMcf = false;

	mcfHandle->setFile(m_szFile.c_str());
	mcfHandle->parseMCF();

	MCFCore::MCFHeaderI* mcfHead = mcfHandle->getHeader();

	DesuraId mcfId = mcfHead->getDesuraId();
	DesuraId itemId =  getItemId();

	validMcf = (mcfHead->isValid() && (!mcfId.isOk() || mcfId == itemId));

	if (!validMcf)
		throw gcException(ERR_INVALIDMCF);


	std::string hash = UTIL::MISC::hashFile(path.getFullPath().c_str());
	uint64 fileSize = UTIL::FS::getFileSize(path);

	char* key = NULL;

	try
	{
		getWebCore()->newUpload(getItemId(), hash.c_str(), fileSize, &key);
	}
	catch (gcException &)
	{
		safe_delete(key);
		throw;
	}

	getUploadManager()->addUpload(getItemId(), key, m_szFile.c_str());
	m_szKey = gcString(key);
	safe_delete(key);

	onCompleteStrEvent(m_szKey);
}

}
}