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
#include "managers/WildcardManager.h"
#include "util_thread/BaseThread.h"



static Thread::Mutex m_WCMutex;

WildcardManager::WildcardManager() : BaseManager<WildcardInfo>( true )
{
	m_uiDepth = 0;
}

WildcardManager::WildcardManager(WildcardManager* mng) : BaseManager<WildcardInfo>( true )
{
	m_uiDepth = 0;

	if (mng)
	{
		for (uint8 x=0; x<mng->getCount(); x++)
		{
			WildcardInfo *temp = dynamic_cast<WildcardInfo*>(mng->getItem(x));

			if (temp)
				addItem(new WildcardInfo(temp->m_szName.c_str(), temp->m_szPath.c_str(), temp->m_szType.c_str(), temp->m_bResolved));
		}

		onNeedSpecialEvent += delegate(&(mng->onNeedSpecialEvent));
	}
}

WildcardManager::~WildcardManager()
{
}


	
void WildcardManager::constructPath(const char* path, char **res, bool fixPath)
{
	if (!path)
		throw gcException(ERR_BADPATH);

	uint8 depth = 0;
	constructPath(path, res, &depth);

	if (fixPath)
	{
		std::string resStr = UTIL::FS::Path(*res, "", false).getFullPath();
		Safe::strcpy(res, resStr.c_str(), resStr.size());
	}
}

void WildcardManager::constructPath(const char* path, char **res, uint8 *depth)
{
	if (!path)
		throw gcException(ERR_BADPATH);

	(*depth)++;

	if (*depth > 25)
		throw gcException(ERR_WILDCARD, "Hit max recursion while constructing path (posible loop).");

	size_t len = strlen(path);

	int32 start = -1;
	int32 stop = 0;

	std::vector<char*> list;
	AutoDeleteV<std::vector<char*>> lad(list);

	//split the string up into section based on %% and then add to vector
	for (size_t x=0; x<len; x++)
	{
		if (path[x] == '%')
		{
			if (x==0)
			{
				start = 0;
			}
			else if (start == -1)
			{
				start = (int32)x;
		
				char* temp = new char[start-stop+1];
				for (int32 y=stop; y<=start; y++)
					temp[y-stop] = path[y];

				temp[start-stop] = '\0';
				list.push_back(temp);
			}
			else
			{
				stop = (int32)x;

				char *temp = new char[stop-start+1];
				for (int32 y = start; y<=stop; y++)
					temp[y-start] = path[y];

				temp[stop-start] = '\0';

				list.push_back(temp);
				start = -1;
				stop++;
			}

		}
		else if (x>=len-1)
		{
			char* temp = new char[len-stop+1];
			for (int32 y=stop; y<(int32)len; y++)
				temp[y-stop] = path[y];

			temp[len-stop] = '\0';

			list.push_back(temp);
		}
	}

	//all those starting with % are wildcards so resolve them
	for (size_t x=0; x<list.size(); x++)
	{
		if (list[x][0] == '%')
		{
			size_t len = strlen(list[x]);
			char *temp = new char[len];
			for (size_t y=1; y<len; y++)
				temp[y-1] = list[x][y];
			temp[len-1]='\0';

			if (strlen(temp)==0)
			{
			  delete [] temp;
				throw gcException(ERR_WILDCARD, gcString("Failed to find wildcard [{0}] Current node is null", path));
			}

			AutoDelete<char> tad(temp);
			AutoDelete<WildcardInfo> wad;

			WildcardInfo* wcInfo = NULL;

			if (Safe::stricmp(temp, "TEMP") == 0)
			{
				WCSpecialInfo info;
				info.name = "temp";
				onNeedInstallSpecialEvent(info);

				if (info.handled)
				{
					wcInfo = new WildcardInfo("temp", info.result.c_str(), "temp", true);
					AutoDelete<WildcardInfo> ad(wcInfo);
					wad = ad;
				}
			}
			else
			{
				wcInfo = findItem(temp);
			}

			if (!wcInfo)
				throw gcException(ERR_WILDCARD, gcString("Failed to find wildcard [{0}]", temp));

			resolveWildCard(wcInfo);

			if (!wcInfo->m_bResolved)
				throw gcException(ERR_WILDCARD, gcString("Failed to resolve wildcard [{0}]", temp));

			if (wcInfo->m_szPath == temp)
			{
				//dont do any thing, dont have enough info yet.
				size_t len = strlen(temp)+3;
				char* newPath = new char[len];
				Safe::snprintf(newPath, len, "%%%s%%", temp);

				safe_delete(list[x]);
				list[x] = newPath;
			}
			else if (wcInfo->m_szPath != "")
			{
				char* newPath = NULL;

				constructPath(wcInfo->m_szPath.c_str(), &newPath, depth);

				safe_delete(list[x]);
				list[x] = newPath;

				if (Safe::stricmp(temp, "TEMP") != 0)
				{
					//this means we dont have to resolve this path next time
					wcInfo->m_szPath = newPath;
				}
			}
			else
			{
				throw gcException(ERR_WILDCARD, gcString("Failed to find wildcard [{0}]", temp));
			}
		}
	}

	size_t totalLen = 0;
	size_t listSize = list.size();

	for (size_t x=0; x<listSize; x++)
		totalLen += strlen(list[x]);

	safe_delete(*res);

	*res = new char[totalLen+1];
	char* cur = *res;

	//put list back into one stting;
	for (size_t x=0; x<listSize; x++)
	{
		size_t itemLen = strlen(list[x]);
		strncpy(cur, list[x], itemLen);
		cur += itemLen;
	}

	(*res)[totalLen] = '\0';
	(*depth)--;
}


uint8 WildcardManager::parseXML(TiXmlNode* node)
{
	if (!node)
		return WCM_ERR_BADXML;

	TiXmlElement* pChild = node->FirstChildElement("wcard");
	while (pChild)
	{
		const char* name = pChild->Attribute("name");
		const char* type = pChild->Attribute("type");
		const char* string = pChild->GetText();
			
		if (name && type && string)
		{
			WildcardInfo *temp = new WildcardInfo(name, string, type);
			addItem(temp);
		}

		pChild = pChild->NextSiblingElement();
	}

	return WCM_OK;
}

void WildcardManager::resolveWildCard(WildcardInfo *wcInfo)
{
	if (wcInfo->m_bResolved)
		return;

	if (wcInfo->m_szType == "path" || wcInfo->m_szType == "exe")
	{
		//if (wildcardCheck(wcInfo->m_szPath.c_str()))
		//	Warning("Wildcard check failed on [%s]\n", string);
		char* path = NULL;

		try
		{
			constructPath(wcInfo->m_szPath.c_str(), &path);
			wcInfo->m_szPath = path;
			wcInfo->m_bResolved = true;
		}
		catch (gcException)
		{
		}

		safe_delete(path);
	}
	else if (wcInfo->m_szType == "regkey")
	{
		wcInfo->m_szPath = UTIL::OS::getConfigValue(wcInfo->m_szPath);
		wcInfo->m_bResolved = true;
	}
	else if (wcInfo->m_szType == "regkey64")
	{
		wcInfo->m_szPath = UTIL::OS::getConfigValue(wcInfo->m_szPath, true);
		wcInfo->m_bResolved = true;
	}
	else if (wcInfo->m_szType == "msicheck" || wcInfo->m_szType == "dotnetcheck")
	{
		WCSpecialInfo info;
		info.name = wcInfo->m_szType;
		info.result = wcInfo->m_szPath;

		onNeedInstallSpecialEvent(info);

		if (info.handled)
		{
			wcInfo->m_szPath = info.result;
			wcInfo->m_bResolved = true;
		}
	}
	else if (wcInfo->m_szType == "special")
	{
		if (wcInfo->m_szName == "INSTALL_PATH" || wcInfo->m_szName == "PARENT_INSTALL_PATH")
		{
			wcInfo->m_bResolved = wcInfo->m_szPath.size() > 0;
		}
		else
		{
			WCSpecialInfo info;
			info.name = wcInfo->m_szName;
						
			needSpecial(&info);

			if (info.handled)
			{
				wcInfo->m_szPath = info.result;
				wcInfo->m_bResolved = true;
			}
		}
	}
	else
	{
		Warning(gcString("Unknown Wildcard type: {0}\n", wcInfo->m_szType));
	}
}

void WildcardManager::updateInstallWildcard(const char* name, const char* value)
{
	if (strcmp(name, "INSTALL_PATH") == 0 || strcmp(name, "PARENT_INSTALL_PATH")==0)
	{
		WildcardInfo* temp = findItem(name);

		if (temp)
		{
			temp->m_szPath = value;
			temp->m_bResolved = true;
		}
		else
		{
			temp = new WildcardInfo(name, value, "special", true);
			addItem(temp);
		}
	}
	else
	{
		Warning(gcString("Trying to update non install wildcard [{0}]\n", name));
	}
}

void WildcardManager::needSpecial(WCSpecialInfo *info)
{
	m_WCMutex.lock();
	onNeedSpecialEvent(*info);
	m_WCMutex.unlock();
}

bool WildcardManager::wildcardCheck(const char* string)
{
	if (!string)
		return false;

	size_t len = strlen(string);

	uint32 count = 0;

	for (size_t x=0; x<len; x++)
	{
		if (string[x] == '%')
			count ++;
	}

	return (count % 2 == 0);
}



void WildcardManager::compactWildCards()
{
	for (size_t x=0; x<getCount(); x++)
	{
		WildcardInfo *temp = dynamic_cast<WildcardInfo*>(getItem((uint32)x));

		if (!temp)
			continue;

		if (wildcardCheck(temp->m_szPath.c_str()))
		{
			char* szPath = NULL;

			try
			{
				constructPath(temp->m_szPath.c_str(), &szPath);
			}
			catch (gcException &)
			{
			}

			safe_delete(szPath);
		}
	}
}
