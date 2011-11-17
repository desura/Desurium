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

#include "UMcfEx.h"
#include "XMLMacros.h"
#include "files.h"



//Downloads update info from web
void UMcfEx::getUpdateInfo(bool saveXml)
{
	ERROR_OUTPUT(__func__);
	HttpHandle wc(PRIMUPDATE);
	
#ifdef NIX64
	wc->addPostText("appid", 120);
#else
	wc->addPostText("appid", 110);
#endif
	wc->postWeb();

	if (wc->getDataSize() == 0)
		throw gcException(ERR_BADRESPONSE, "Failed to get update data. Desura webserver might be down or you are not connected to the internet.");
	
	TiXmlDocument doc;
	doc.LoadBuffer(const_cast<char*>(wc->getData()), wc->getDataSize());

	XML::processStatus(doc, "appupdate");
	
	parseUpdateXml(doc);

	if (m_pFileList.size() == 0)
		throw gcException(ERR_INVALIDDATA, "The update mcf has no files.");

	if (saveXml)
		doc.SaveFile(UPDATEXML);
}

void UMcfEx::downloadMcf()
{
	ERROR_OUTPUT(__func__);
	uint32 prog = 0;
	onProgressEvent(prog);

	if (!m_szUrl.c_str())
		throw gcException(ERR_INVALIDDATA, "The update url is invalid");

	if (!m_szFile.c_str())
		throw gcException(ERR_INVALIDDATA, "The mcf save path is invalid");

	HttpHandle wc(m_szUrl.c_str());
	wc->setOutFile(gcString(m_szFile).c_str());
	wc->getProgressEvent() += delegate((UMcf*)this, &UMcf::updateAllCB);

	int res = wc->getWebToFile();

	if (res == UWEB_USER_ABORT)
		throw gcException(ERR_USERCANCELED, "The user canceled the download");
	else if (res != 0)
		throw gcException(ERR_LIBCURL, "Failed to download mcf file");
	
	m_sHeader->setBuild(MCFBuild::BuildFromInt(m_iAppBuild));
	m_sHeader->setId(m_iAppId);
}

void UMcfEx::deleteMcf()
{
	DeleteFile(gcString(m_szFile).c_str());
}

void UMcfEx::checkMcf()
{
	HttpHandle wc(m_szUrl.c_str());
	
	wc->setDownloadRange(0, 5);
	wc->getWeb();
	
	if (wc->getDataSize() == 0)
		throw gcException(ERR_BADRESPONSE, "Failed to get update data. Desura webserver might be down or you are not connected to the internet.");
	
	if (strncmp(wc->getData(), "LMCF", 4)==0)
		return;
	
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
		throw gcException(ERR_BADRESPONSE, std::string(start, len+1));
	}

	throw gcException(ERR_BADRESPONSE, "Unknown response");
}
