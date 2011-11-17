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

#include "StdAfx.h"
#include "Common.h"
#include "UMcfEx.h"


UMcfEx::UMcfEx()
{
	m_szLastError = NULL;
	m_uiLevel = 0;
}

UMcfEx::~UMcfEx()
{
	safe_delete(m_szLastError);
}

//Downloads update info from web
void UMcfEx::getUpdateInfo(bool saveXml)
{
	HttpHandle wc(PRIMUPDATE);
	wc->getWeb();

	if (wc->getDataSize() == 0)
	{
		::MessageBox(NULL, "Failed to get update data.\n\nDesura webserver might be down or you are not connected to the internet.", "Desura: ERROR", MB_OK);
		exit(-1);
	}

	TiXmlDocument doc;
	doc.LoadBuffer(const_cast<char*>(wc->getData()), wc->getDataSize());

	parseUpdateXml(doc);

	if (m_pFileList.size() == 0)
	{
		::MessageBox(NULL, "Failed to get update files.\n\nEther Desura download server is down or the update xml feed is bad.", "Desura: ERROR", MB_OK);
		exit(-1);
	}

	if (saveXml)
		doc.SaveFile(UPDATEXML);
}

void UMcfEx::downloadMcf()
{
	uint32 prog = 0;
	onProgressEvent(prog);

	if (m_szUrl == "")
	{
		::MessageBox(NULL, "Invalid Download URL.", "Desura: Error", MB_OK);
		exit(-1);
	}

	if (m_szFile == L"")
	{
		::MessageBox(NULL, "Invalid MCF file path.", "Desura: Error", MB_OK);
		exit(-1);
	}

	HttpHandle wc(m_szUrl.c_str());
	wc->setOutFile(gcString(m_szFile).c_str());
	wc->getProgressEvent() += delegate((UMcf*)this, &UMcf::updateAllCB);
	
	try
	{
		wc->getWebToFile();
	}
	catch (gcException &e)
	{
		char msg[255];
		_snprintf_s(msg, 255, _TRUNCATE, "Failed to download Desura Files. \n\n%s [%d.%d]", e.getErrMsg(), e.getErrId(), e.getSecErrId());
		::MessageBox(NULL, msg, "Desura: Error", MB_OK);
		exit(-1);
	}

	m_sHeader->setBuild(MCFBuild::BuildFromInt(m_iAppBuild));
	m_sHeader->setId(m_iAppId);
}

void UMcfEx::deleteMcf()
{
	if (m_szFile == L"")
		return;

	DeleteFileW(m_szFile.c_str());
}

uint32 UMcfEx::progressUpdate(Prog_s *info, uint32 other)
{
	uint32 prog = UMcf::progressUpdate(info, other);

	if (m_uiLevel == 0)
	{
		return prog;
	}
	else if (m_uiLevel == 1 && prog != 0)
	{
		return prog*9/10;
	}
	else if (m_uiLevel == 3)
	{
		if (prog != 0)
			return 90 + prog/10;
		else
			return 90;
	}

	return 0;
}

bool UMcfEx::checkMcf()
{
	HttpHandle wc(m_szUrl.c_str());
	wc->setDownloadRange(0, 5);

	try
	{
		wc->getWeb();
	}
	catch (gcException &e)
	{
		Safe::strcpy(&m_szLastError, e.getErrMsg(), 255);
		return false;
	}

	if (wc->getDataSize() == 0)
	{
		Safe::strcpy(&m_szLastError, "Failed to download file from web.", 255);
		return false;
	}
	else if (strncmp(wc->getData(), "LMCF", 4)==0)
	{
		return true;
	}
	else
	{
		//check to see if we had a http error
		wc->cleanUp();
		wc->setDownloadRange(0, 1000);
		wc->getWeb();

		const char* title =strstr(wc->getData(), "<title>");
		
		if (title)
		{
			const char *start = title+7;
			const char *end = strstr(start, "</title>");

			size_t len = end - start;
			safe_delete(m_szLastError);

			m_szLastError = new char[len+1];
			strncpy_s(m_szLastError, len+1, start, len);
			m_szLastError[len]='\0';
			return false;
		}
	}

	Safe::strcpy(&m_szLastError, "Unknown Error.", 255);
	return false;
}