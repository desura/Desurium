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
#include "UploadThread.h"

#include <algorithm>

#define MINCHUNKSIZE (500*1024)

namespace UserCore
{
namespace Thread
{


UploadThread::UploadThread(UploadThreadInfo* info) : MCFThread( "Upload Thread", info->itemId )
{
	m_pInfo = info;

	m_uiContinueCount = 0;
	m_uiChunkSize = 1024*1024; //chunksize;
	m_uiFileSize = 0;
	m_uiAmountRead =0;

	m_fLastAmmount = 0;
	m_bSetPauseStart = false;
	m_bCancel = false;
}

UploadThread::~UploadThread()
{
	stop();
}

void UploadThread::doRun()
{
	DesuraId id= getItemId();
	gcString type = id.getTypeString();


	uint32 sCode = 0;
	UTIL::FS::Path path = UTIL::FS::PathWithFile(m_pInfo->szFile);
	m_uiFileSize = UTIL::FS::getFileSize(path);

	if (m_uiFileSize == 0)
		throw gcException(ERR_ZEROFILE, gcString("File size is zero [{0}].", m_pInfo->szFile));

	UTIL::FS::FileHandle hFile(path, UTIL::FS::FILE_READ);

	if (m_pInfo->uiStart > 0)
	{
		hFile.seek(m_pInfo->uiStart);

		UserCore::Misc::UploadInfo ui;
		ui.percent = (uint8)(m_pInfo->uiStart*100/m_uiFileSize);
		onUploadProgressEvent(ui);

		ui = UserCore::Misc::UploadInfo();
		ui.milestone = true;
		onUploadProgressEvent(ui);
	}

	uint64 chunkCalc = m_uiFileSize / 100;
	//max chunck size is 40mg, min is 5mg

	uint32 minSize = 5*1024*1024;
	uint32 maxSize = 40*1024*1024;

	if (chunkCalc > UINT_MAX)
		m_uiChunkSize = maxSize;
	else
		m_uiChunkSize = Clamp((uint32)chunkCalc, minSize, maxSize);

	//if the file is 20% (or less) bigger than one chunk. Upload in one go
	if ( (uint64)((double)m_uiChunkSize*1.2) > m_uiFileSize)
		m_uiChunkSize = (uint32)m_uiFileSize;

	gcString url = getWebCore()->getUrl(WebCore::McfUpload);
	m_hHttpHandle->setUrl(url.c_str());

	UTIL::MISC::Buffer buffer(m_uiChunkSize);

	while (sCode != 999)
	{
		if (m_bCancel)
			break;

		if (m_bSetPauseStart)
		{
			m_tPauseStartTime = boost::posix_time::ptime(boost::posix_time::second_clock::universal_time());
			m_bSetPauseStart = false;
		}

		while (isPaused())
		{
			gcSleep(500);
		}

		uint32 chunkSize = m_uiChunkSize;
		uint64 fileLeft =  m_uiFileSize - (m_uiAmountRead + m_pInfo->uiStart);
		if ((uint64)chunkSize > fileLeft)
		{
			chunkSize = (uint32)(fileLeft);
		}

		if (sCode == 0)
		{
			hFile.read(buffer, chunkSize);
			sCode = 900;
		}


		m_hHttpHandle->cleanUp();
		m_hHttpHandle->clearCookies();
		getWebCore()->setWCCookies(m_hHttpHandle);

		m_hHttpHandle->addPostText("key", m_pInfo->szKey.c_str());
		m_hHttpHandle->addPostText("action", "uploadchunk");
		m_hHttpHandle->addPostText("siteareaid", id.getItem());
		m_hHttpHandle->addPostText("sitearea", type.c_str());
		m_hHttpHandle->addPostFileAsBuff("mcf", "upload.mcf", buffer, chunkSize);
		m_hHttpHandle->addPostText("uploadsize", chunkSize);
		m_hHttpHandle->getProgressEvent()  += delegate(this,  &UploadThread::onProgress);


		if (m_tStartTime.is_not_a_date_time())
			m_tStartTime = boost::posix_time::ptime(boost::posix_time::second_clock::universal_time());

		uint8 res = 0;

		//need to check here a second time incase we where paused or by fluke missed the check the first time as m_hHttpHandle->CleanUp() removes the abort flag.
		if (m_bCancel)
			break;

		try
		{
			res = m_hHttpHandle->postWeb();
		}
		catch (gcException &except)
		{
			if (m_uiContinueCount > 3)
			{
				throw;
			}
			else
			{
				m_uiContinueCount++;
				Warning(gcString("Failed to upload chunk: {0}. Retrying.\n", except));	
				continue;
			}
		}

		if (res == UWEB_USER_ABORT)
			continue;

		const char* error = m_hHttpHandle->getData();

		//Warning("UC: %s\n", error);

		TiXmlDocument doc;
		doc.LoadBuffer(const_cast<char*>(error), m_hHttpHandle->getDataSize());

		TiXmlNode *gNode = doc.FirstChild("itemupload");
		if (!gNode)
		{
			if (m_uiContinueCount > 3)
			{
				throw gcException(ERR_BADXML, "Couldnt find the root node in itemupload xml.");
			}
			else
			{
				m_uiContinueCount++;
				Warning(gcString("Unable to find root node for upload! Retrying. [{0}]\n", m_pInfo->szFile));
				continue;
			}
		}

		TiXmlNode *gMsg = gNode->FirstChild("status");
		if (!gMsg)
		{
			if (m_uiContinueCount > 3)
			{
				throw gcException(ERR_BADXML, "Couldnt find the status node in itemupload xml.");
			}
			else
			{
				m_uiContinueCount++;
				Warning(gcString("Unable to find status node for file upload. Retrying. [{0}]\n", m_pInfo->szFile));
				continue;
			}
		}
		
		TiXmlElement* gEle = gMsg->ToElement();

		const char* id = gEle->Attribute("code");
		const char* msg = gEle->GetText();

		if (id)
		{
			sCode = atoi(id);

			if (sCode == 0 || sCode == 999)
			{
				m_uiAmountRead += chunkSize;
				m_uiContinueCount = 0;

				UserCore::Misc::UploadInfo ui;
				ui.milestone = true;
				onUploadProgressEvent(ui);
			}
			else if (sCode == 107 && m_uiContinueCount < 3)	//only part upload
			{
				Warning(gcString("Upload failed, bad status: {0} [{1}]. Trying to continue.\n", sCode, msg));
				m_uiContinueCount++;
				continue;
			}
			else
			{	
				throw gcException(ERR_BADSTATUS, gcString("Upload failed, bad status: {0} [{1}]", sCode, msg));
				break;
			}
		}
		else
		{
			if (m_uiContinueCount > 3)
			{
				throw gcException(ERR_BADXML, "Couldnt find the id node in itemupload xml.");
			}
			else
			{
				m_uiContinueCount++;
				//bad upload, let it continue restart it
				Warning(gcString("Unable to find id node for file upload. Retrying. [{0}]\n", m_pInfo->szFile));	
			}
		}
	}

	hFile.close();

	if (sCode == 999)
	{
		onCompleteEvent(sCode);
		getUploadManager()->removeUpload(m_pInfo->szKey.c_str(), false);
	}
}


void UploadThread::onProgress(Prog_s& p)
{
	Prog_s* temp = &p;

	if (!temp)
		return;

	uint64 currProg = m_uiAmountRead + (uint64)temp->ulnow;

	UserCore::Misc::UploadInfo ui;
	ui.num = 0;
	ui.totalAmmount = m_uiFileSize;
	ui.doneAmmount = currProg + m_pInfo->uiStart;
	ui.percent = (uint8)((currProg+m_pInfo->uiStart)*100/m_uiFileSize);

	if (temp->abort)
	{
		ui.doneAmmount = m_uiAmountRead + m_pInfo->uiStart;
		ui.paused = true;
		onUploadProgressEvent(ui);
		return;
	}

	boost::posix_time::ptime curTime(boost::posix_time::second_clock::universal_time());

	double rate = (temp->ulnow - m_fLastAmmount)/((double)(curTime - m_tLastTime).total_seconds());
	
	if (m_fLastAmmount < 1)
		rate = 0;

	m_fLastAmmount = temp->ulnow;
	m_tLastTime = curTime;

	boost::posix_time::time_duration total = curTime - m_tStartTime;
	total -= m_tTotPauseTime;

	double avgRate = (currProg) / ((double)total.total_seconds());
	double pred = (m_uiFileSize - currProg - m_pInfo->uiStart) / avgRate;

	boost::posix_time::time_duration predTime = boost::posix_time::seconds((long)pred);

	ui.min = (uint8)predTime.minutes();
	ui.hour = (uint8)predTime.hours();
	ui.rate = (uint32)(avgRate);

	onUploadProgressEvent(ui);
}

void UploadThread::onPause()
{
	m_bSetPauseStart = true;
	m_hHttpHandle->abortTransfer();

	onPauseEvent();
}

void UploadThread::onUnpause()
{
	boost::posix_time::ptime curTime(boost::posix_time::second_clock::universal_time());
	m_tTotPauseTime += curTime - m_tPauseStartTime;

	onUnpauseEvent();
}

void UploadThread::onStop()
{
	m_bCancel = true;
	m_hHttpHandle->abortTransfer();
}

}
}
