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
#include "WebCore.h"

#include "XMLMacros.h"

#include "sqlite3x.hpp"
#include "sql/WebCoreSql.h"



namespace WebCore
{


TiXmlNode* WebCoreClass::postToServer(std::string url, std::string resource, PostMap &postData, TiXmlDocument &doc, bool useHTTPS)
{
	gcString httpOut;

	{
		HttpHandle hh(url.c_str(), useHTTPS);

		if (useHTTPS)
		{
			hh->setUserAgent(getUserAgent());
			hh->setCertFile(
				UTIL::STRING::toStr(UTIL::OS::getDataPath(L"ca-bundle.crt")).c_str());
		}
		else
		{
			setWCCookies(hh);
		}

		PostMap::iterator it = postData.begin();

		while (it != postData.end())
		{
			hh->addPostText(it->first.c_str(), it->second.c_str());
			it++;
		}

		hh->postWeb();
	
		if (hh->getDataSize() == 0)
			throw gcException(ERR_BADRESPONSE, "Data size was zero");

		XML::loadBuffer(doc, const_cast<char*>(hh->getData()), hh->getDataSize());

		if (m_bDebuggingOut)
			httpOut.assign(const_cast<char*>(hh->getData()), hh->getDataSize());
	}

	TiXmlNode *uNode = doc.FirstChild(resource.c_str());

	if (m_bDebuggingOut && !uNode)
		Warning(httpOut);

	XML::processStatus(doc, resource.c_str());
	return uNode;
}

TiXmlNode* WebCoreClass::loginToServer(std::string url, std::string resource, PostMap &postData, TiXmlDocument &doc)
{
	return postToServer(url, resource, postData, doc, true);
}

DesuraId WebCoreClass::nameToId(const char* name, const char* type)
{
	if (!name)
		throw gcException(ERR_BADITEM, "The name is NULL");

	gcString key("{0}-{1}", name, type);
	uint32 hash = UTIL::MISC::RSHash_CSTR(key.c_str());

	try 
	{
		sqlite3x::sqlite3_connection db(getWebCoreDb(m_szAppDataPath.c_str()).c_str());
		gcString q("select internalid from namecache where nameid='{0}' and ttl > DATETIME('NOW');",  hash);

		DesuraId id(db.executeint64(q.c_str()));
		if (id.isOk())
			return id;
	}
	catch(std::exception &) 
	{
	}

	TiXmlDocument doc;
	PostMap post;

	post["nameid"] = name;
	post["sitearea"] = type;

	TiXmlNode *uNode = postToServer(getNameLookUpUrl(), "iteminfo", post, doc);
	TiXmlNode* cNode = uNode->FirstChild("item");

	if (cNode)
	{
		TiXmlElement* cEl = cNode->ToElement();
		
		if (cEl)
		{
			const char* idStr = cEl->Attribute("siteareaid");
			const char* typeS = cEl->Attribute("sitearea");

			DesuraId id(idStr, typeS);

			if (!id.isOk() || DesuraId::getTypeString(id.getType()) != type)
			{
				throw gcException(ERR_BADXML);
			}
			else
			{
				try 
				{
					sqlite3x::sqlite3_connection db(getWebCoreDb(m_szAppDataPath.c_str()).c_str());
					gcString q("replace into namecache (internalid, nameid, ttl) values ('{0}','{1}', DATETIME('NOW', '+5 day'));", id.toInt64(), hash);
					db.executenonquery(q.c_str());
				}
				catch(std::exception &ex) 
				{
					Warning(gcString("Failed to update namecache in webcore: {0}\n", ex.what()));
				}	

				return id;
			}
		}
	}

	throw gcException(ERR_BADXML);
}

DesuraId WebCoreClass::hashToId(const char* itemHashId)
{
	if (!itemHashId)
		throw gcException(ERR_BADITEM, "The hash is NULL");


	try 
	{
		sqlite3x::sqlite3_connection db(getWebCoreDb(m_szAppDataPath.c_str()).c_str());
		gcString q("select internalid from namecache where hashid='{0}' and ttl > DATETIME('NOW');", UTIL::MISC::RSHash_CSTR(itemHashId) );
		DesuraId id(db.executeint(q.c_str()));

		if (id.isOk())
			return id;
	}
	catch(std::exception &) 
	{
	}


	TiXmlDocument doc;
	PostMap post;

	post["hashid"] = itemHashId;

	TiXmlNode *uNode = postToServer(getNameLookUpUrl(), "iteminfo", post, doc);
	TiXmlNode* cNode = uNode->FirstChild("item");

	if (cNode)
	{
		TiXmlElement* cEl = cNode->ToElement();
		
		if (cEl)
		{
			const char* idStr = cEl->Attribute("siteareaid");
			const char* typeS = cEl->Attribute("sitearea");

			if (!typeS || !idStr)
			{
				throw gcException(ERR_BADXML);
			}
			else
			{
				DesuraId id(idStr, typeS);

				try 
				{
					sqlite3x::sqlite3_connection db(getWebCoreDb(m_szAppDataPath.c_str()).c_str());
					gcString q("replace into namecache (internalid, hashid, ttl) values ('{0}','{1}', DATETIME('NOW', '+5 day'));", id.toInt64(), UTIL::MISC::RSHash_CSTR(itemHashId));
					db.executenonquery(q.c_str());
				}
				catch(std::exception &ex) 
				{
					Warning(gcString("Failed to update namecache in webcore: {0}\n", ex.what()));
				}	

				return id;
			}
		}
	}

	throw gcException(ERR_BADXML);
}


void WebCoreClass::updateAccountItem(DesuraId id, bool add)
{
	TiXmlDocument doc;
	PostMap post;

	post["siteareaid"] = id.getItem();
	post["sitearea"] = id.getTypeString();
	post["action"] = add?"add":"delete";

	postToServer(getUpdateAccountUrl(), "iteminstall", post, doc);
}

void WebCoreClass::newUpload(DesuraId id, const char* hash, uint64 fileSize, char **key)
{
	gcString size("{0}", fileSize);

	TiXmlDocument doc;
	PostMap post;

	post["siteareaid"] = id.getItem();
	post["sitearea"] = id.getTypeString();
	post["action"] = "newupload";
	post["filehash"] = hash;
	post["filesize"] = size;

	TiXmlNode *uNode = postToServer(getMcfUploadUrl(), "itemupload", post, doc);
	TiXmlNode* iNode = uNode->FirstChild("mcf");
	
	if (!iNode)
		throw gcException(ERR_BADXML);	

	TiXmlElement* cEl = iNode->ToElement();
		
	if (cEl)
	{
		const char* text = cEl->Attribute("key");

		if (!text)
			throw gcException(ERR_BADXML);	

		Safe::strcpy(key, text, 255);
	}
}



void WebCoreClass::resumeUpload(DesuraId id, const char* key, WebCore::Misc::ResumeUploadInfo &info)
{
	TiXmlDocument doc;
	PostMap post;

	post["siteareaid"] = id.getItem();
	post["sitearea"] = id.getTypeString();
	post["action"] = "resumeupload";
	post["key"] = key;

	TiXmlNode *uNode = postToServer(getMcfUploadUrl(), "itemupload", post, doc);
	TiXmlNode* mNode = uNode->FirstChild("mcf");

	if (!mNode)
		throw gcException(ERR_BADXML);	

	gcString complete;
	XML::GetChild("complete", complete, mNode);

	if (complete == "1")
		throw gcException(ERR_COMPLETED);

	XML::GetChild("date", info.szDate, mNode);
	XML::GetChild("filehash", info.szHash, mNode);
	XML::GetChild("filesize", info.size, mNode);
	XML::GetChild("filesizeup", info.upsize, mNode);
}


void WebCoreClass::getItemInfo(DesuraId id, TiXmlDocument &doc, MCFBranch mcfBranch, MCFBuild mcfBuild)
{
	PostMap post;

	post["siteareaid"] = id.getItem();
	post["sitearea"] = id.getTypeString();

	if (mcfBuild != 0)
		post["build"] = mcfBuild;

	if (mcfBranch != 0)
	{
		if (mcfBranch.isGlobal())
			post["branchglobal"] = mcfBranch;
		else
			post["branch"] = mcfBranch;
	}

	postToServer(getItemInfoUrl(), "iteminfo", post, doc);
}

gcString WebCoreClass::getCDKey(DesuraId id, MCFBranch branch)
{
	TiXmlDocument doc;
	PostMap post;

	post["siteareaid"] = id.getItem();
	post["sitearea"] = id.getTypeString();
	post["branch"] = (size_t)branch;
	
#ifdef WIN32
	post["token"] =  UTIL::WIN::getRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography\\MachineGuid", true);
#else
	post["token"] = "todo";
#endif

	TiXmlNode* root = postToServer(getCDKeyUrl(), "cdkey", post, doc);
	TiXmlElement* key = root->FirstChildElement("key");

	if (!key)
		throw gcException(ERR_BADXML);

	return key->GetText();
}

void WebCoreClass::logIn(const char* user, const char* pass, TiXmlDocument &doc)
{
	if (m_bUserAuth)
		throw gcException(ERR_ALREADYLOGGEDIN);

	PostMap post;

	post["username"] = user;
	post["password"] = pass;

	TiXmlNode* uNode = loginToServer(getLoginUrl(), "memberlogin", post, doc);
	TiXmlElement *memNode = uNode->FirstChildElement("member");
	
	if (!memNode)
		throw gcException(ERR_BADXML);

	const char* idStr =  memNode->Attribute("siteareaid");

	if (!idStr || atoi(idStr) < 0)
		throw gcException(ERR_BAD_PORU);

	m_uiUserId = atoi(idStr);

	TiXmlNode *cookieNode = memNode->FirstChild("cookies");
	if (cookieNode)
	{
		XML::GetChild("id", m_szIdCookie, cookieNode);
		XML::GetChild("session", m_szSessCookie, cookieNode);
	}

	m_bUserAuth = true;
}

void WebCoreClass::logOut()
{
	m_bUserAuth = false;
	m_szIdCookie = gcString("");
	m_szSessCookie = gcString("");
}

void WebCoreClass::getUpdatePoll(TiXmlDocument &doc, const std::map<std::string, std::string> &post)
{
	PostMap postData;

	std::for_each(post.begin(), post.end(), [&postData](std::pair<std::string, std::string> p)
	{
		postData[p.first] = p.second;
	});

	postToServer(getUpdatePollUrl(), "updatepoll", postData, doc);
}

void WebCoreClass::getLoginItems(TiXmlDocument &doc)
{
	PostMap postData;
	postToServer(getMemberDataUrl(), "memberdata", postData, doc);
}

}
