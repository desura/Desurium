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
#include "MCF.h"

#include "mcfcore/DownloadProvider.h"
#include "mcfcore/UserCookies.h"
#include "XMLMacros.h"

#include "thread/HGTController.h"
#include "BZip2.h"




namespace MCFCore
{

void MCF::getDownloadProviders(const char* url, MCFCore::Misc::UserCookies *pCookies, bool *unauthed, bool local)
{
	if (!pCookies)
		throw gcException(ERR_INVALID, "Cookies are null (getDownloadProviders)");

	HttpHandle wc(url);
	pCookies->set(wc);


	DesuraId id = m_sHeader->getDesuraId();
	gcString type = id.getTypeString();

	wc->addPostText("siteareaid", id.getItem());
	wc->addPostText("sitearea", type.c_str());
	wc->addPostText("branch", m_sHeader->getBranch());

	MCFBuild build = m_sHeader->getBuild();

	if (build != 0)
		wc->addPostText("build", build);

	if (local)
		wc->addPostText("local", "yes");


	wc->postWeb();

	if (wc->getDataSize() == 0)
		throw gcException(ERR_BADRESPONSE);
	
	TiXmlDocument doc;

	doc.SetCondenseWhiteSpace(false);
	XML::loadBuffer(doc, const_cast<char*>(wc->getData()), wc->getDataSize());

	TiXmlNode *uNode = doc.FirstChild("itemdownloadurl");

	if (!uNode)
		throw gcException(ERR_BADXML);

	TiXmlNode* sNode = uNode->FirstChild("status");

	if (!sNode)
		throw gcException(ERR_BADXML);

	uint32 status = 0;
	TiXmlElement* sEl = sNode->ToElement();
	if (sEl)
	{
		const char* statStr = sEl->Attribute("code");
		if (statStr)
			status = atoi(statStr);
		else
			throw gcException(ERR_BADXML);
	}
	else
	{
		throw gcException(ERR_BADXML);
	}

	if (status != 0)
		throw gcException(ERR_BADSTATUS, status, gcString("Status: {0}", sEl->GetText()));

	TiXmlNode* iNode = uNode->FirstChild("item");

	if (!iNode)
	{
		throw gcException(ERR_BADXML);
	}

	TiXmlNode* mNode = iNode->FirstChild("mcf");

	if (!mNode)
	{
		throw gcException(ERR_BADXML);
	}

	TiXmlElement *melNode = mNode->ToElement();

	if (melNode)
	{
		const char* build = melNode->Attribute("build");
		const char* branch = melNode->Attribute("branch");

		//Debug(gcString("MCF: R: {0}.{1} G: {2}.{3}\n", m_sHeader->getBranch(), m_sHeader->getBuild(), build, branch));

		if (build)
			m_sHeader->setBuild(MCFBuild::BuildFromInt(atoi(build)));

		if (branch)
			m_sHeader->setBranch(MCFBranch::BranchFromInt(atoi(branch)));
	}
	
	safe_delete(m_pFileAuth);
	
	char *szAuthCode = NULL;
	XML::GetChild("authhash", szAuthCode, mNode);
	if (!szAuthCode)
		throw gcException(ERR_BADXML);

	m_pFileAuth = new Misc::GetFile_s;
	memset(m_pFileAuth, 0, sizeof(Misc::GetFile_s));

	Safe::strncpy(m_pFileAuth->authhash, 33, szAuthCode, 33);

	char buff[10];
	Safe::snprintf(buff, 10, "%d", pCookies->getUserId());
	size_t size = Safe::strlen(buff, 10);
	
	memcpy(m_pFileAuth->authkey, buff, size);
	safe_delete(szAuthCode);

	TiXmlNode* urlNode = mNode->FirstChild("urls");

	if (!urlNode)
		throw gcException(ERR_BADXML);

	char * szAuthed = NULL;
	XML::GetChild("authed", szAuthed, mNode);

	if (unauthed && szAuthed)
	{
		*unauthed = (atoi(szAuthed) == 0);
	}

	safe_delete(szAuthed);


#ifdef DEBUG
#if 0
	m_vProviderList.clear();
	m_vProviderList.push_back(new MCFCore::Misc::DownloadProvider("localhost", "mcf://10.0.0.121:62001", "", ""));
	return;
#endif
#endif

	TiXmlElement* pChild = urlNode->FirstChildElement("url");
	while (pChild)
	{
		MCFCore::Misc::DownloadProvider* temp = new MCFCore::Misc::DownloadProvider(pChild);
		m_vProviderList.push_back(temp);
		pChild = pChild->NextSiblingElement();
	}

	if (m_vProviderList.size() == 0)
		throw gcException(ERR_ZEROSIZE);
}

void MCF::dlHeaderFromHttp(const char* url)
{
	if (m_bStopped)
		return;

	if (!url)
		throw gcException(ERR_BADURL);

	//FIXME: Needs error checking on getweb
	HttpHandle wc(url);
	wc->setDownloadRange(0, MCFCore::MCFHeader::getSizeS());

	wc->getWeb();

	if (wc->getDataSize() != MCFCore::MCFHeader::getSizeS())
		throw gcException(ERR_BADHEADER);

	MCFCore::MCFHeader webHeader((uint8*)wc->getData());
	setHeader(&webHeader);
	
	if (!webHeader.isValid())
		throw gcException(ERR_BADHEADER);

	wc->cleanUp();
	wc->setDownloadRange(webHeader.getXmlStart(), webHeader.getXmlSize());
	wc->getWeb();

	if (wc->getDataSize() == 0 || wc->getDataSize() != webHeader.getXmlSize())
		throw gcException(ERR_WEBDL_FAILED, "Failed to download MCF xml from web (size is ether zero or didnt match header size)");

	uint32 bz2BuffLen = webHeader.getXmlSize()*25;
	char* bz2Buff = NULL;

	if ( isCompressed() )
	{
		bz2Buff = new char[bz2BuffLen];
		UTIL::STRING::zeroBuffer(bz2Buff, bz2BuffLen);

		try
		{
			UTIL::BZIP::BZ2DBuff((char*)bz2Buff, &bz2BuffLen, const_cast<char*>(wc->getData()), wc->getDataSize());
			parseXml(bz2Buff, bz2BuffLen);
			safe_delete(bz2Buff);
		}
		catch (gcException &)
		{
			safe_delete(bz2Buff);
			throw;
		}
	}
	else
	{
		parseXml(const_cast<char*>(wc->getData()), wc->getDataSize());
	}

	//we remove the complete flag due to the files not existing in the MCF
	for (size_t x=0; x< m_pFileList.size(); x++)
	{
		m_pFileList[x]->delFlag(MCFCore::MCFFileI::FLAG_COMPLETE);
	}
}

void MCF::dlFilesFromHttp(const char* url, const char* installDir)
{
	assert(!m_pTHandle);

	if (m_bStopped)
		return;

	if (!url)
		throw gcException(ERR_BADURL);

	//save the header first incase we fail
	saveMCF_Header();

	MCFCore::Thread::HGTController *temp = new MCFCore::Thread::HGTController(url, this, installDir);
	temp->onProgressEvent +=delegate(&onProgressEvent);
	temp->onErrorEvent += delegate(&onErrorEvent);

	m_pTHandle = temp;

	try
	{
		m_pTHandle->start();
		m_pTHandle->join();
		safe_delete(m_pTHandle);
	}
	catch (gcException &)
	{
		safe_delete(m_pTHandle);
		throw;
	}

	saveMCF_Header();
}

}