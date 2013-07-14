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
#include "InstallInfo.h"

#include "XMLMacros.h"

namespace UserCore
{
namespace Misc
{

InstallInfo::InstallInfo(DesuraId id, DesuraId pid)
:	m_bInstalled(false),
	m_iID(id),
	m_iParentID(pid)
{}

InstallInfo::~InstallInfo()
{
}

void InstallInfo::loadXmlData(tinyxml2::XMLNode *xmlNode, WildcardManager* pWildCard)
{
	WildcardManager lwc(pWildCard);

	tinyxml2::XMLElement* wcNode = xmlNode->FirstChildElement("wcards");
	if (wcNode)
	{
		lwc.parseXML(wcNode);
	}

	XML::GetChild("name", m_szName, xmlNode);
	tinyxml2::XMLElement* setNode = xmlNode->FirstChildElement("settings");
	if (setNode)
	{
		tinyxml2::XMLElement* icsNode = setNode->FirstChildElement("installlocations");
		if (icsNode)
		{
			tinyxml2::XMLElement* icNode = icsNode->FirstChildElement("installlocation");

			while (icNode)
			{
				char* path = NULL;
				char* check = NULL;

				XML::GetChild("path", path, icNode);
				XML::GetChild("check", check, icNode);

				if (!path || !check)
				{
					safe_delete(path);
					safe_delete(check);
					continue;
				}

				char* CheckRes = NULL;
				char* PathRes = NULL;

				try
				{
					lwc.constructPath(check, &CheckRes);
					lwc.constructPath(path, &PathRes);

					safe_delete(path);
					safe_delete(check);

					if (CheckRes && PathRes && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(CheckRes)))
					{
						m_szPath = PathRes;
						m_bInstalled = true;
						safe_delete(CheckRes);
						safe_delete(PathRes);
						break;
					}
				}
				catch (gcException &e)
				{
					Debug(gcString("InstallInfo: Error parsing wildcards for installInfo: {0}\n", e));
				}

				safe_delete(path);
				safe_delete(check);
				safe_delete(CheckRes);
				safe_delete(PathRes);

				icNode = icNode->NextSiblingElement();
			}
		}
	}
}

}
}
