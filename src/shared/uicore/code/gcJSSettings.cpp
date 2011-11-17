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
#include "managers/CVar.h"
#include "gcJSSettings.h"

#include "MainApp.h"

#define STEAMPATH "HKEY_CURRENT_USER\\Software\\Valve\\Steam\\SteamPath"
#define NOSTEAM "Steam Not Installed"

void RestartAndSetMCFCache(const char* dir);

#include <wx/filepicker.h>
#include "usercore/CIPManagerI.h"

#ifdef NIX
#include "util/UtilLinux.h"
#endif

REGISTER_JSEXTENDER(DesuraJSSettings);

DesuraJSSettings::DesuraJSSettings() : DesuraJSBase("settings", "native_binding_settings.js")
{
	REG_SIMPLE_JS_FUNCTION( getValue,	DesuraJSSettings );
	REG_SIMPLE_JS_FUNCTION( getCoreCount,		DesuraJSSettings );

	REG_SIMPLE_JS_FUNCTION( getLanguages, DesuraJSSettings );
	REG_SIMPLE_JS_FUNCTION( getSteamNames,	DesuraJSSettings );
	REG_SIMPLE_JS_FUNCTION( getThemes, DesuraJSSettings );

	REG_SIMPLE_JS_VOIDFUNCTION( setValue, DesuraJSSettings );

	REG_SIMPLE_JS_FUNCTION( getCurrentCIPItems, DesuraJSSettings );
	REG_SIMPLE_JS_FUNCTION( getAllCIPItems,	DesuraJSSettings );
	REG_SIMPLE_JS_FUNCTION( isValidCIPPath, DesuraJSSettings );
	REG_SIMPLE_JS_FUNCTION( browseCIPPath, DesuraJSSettings );

	REG_SIMPLE_JS_VOIDFUNCTION( updateCIPList,		DesuraJSSettings );
	REG_SIMPLE_JS_VOIDFUNCTION( saveCIPList, DesuraJSSettings );

	REG_SIMPLE_JS_FUNCTION( isValidLinkBinary, DesuraJSSettings );
	REG_SIMPLE_JS_FUNCTION( browseLinkBinary, DesuraJSSettings );
}



gcString DesuraJSSettings::getValue(gcString name)
{
	CVar* cvar = GetCVarManager()->findCVar(name.c_str());
	if (!cvar)
		return "";

	return cvar->getExitString();
}

int32 DesuraJSSettings::getCoreCount()
{
	return UTIL::MISC::getCoreCount();
}

void DesuraJSSettings::setValue(gcString name, gcString val)
{
	CVar* cvar = GetCVarManager()->findCVar(name.c_str());

	if (cvar && val != gcString(cvar->getString()))
		cvar->setValue(val.c_str());
}

std::vector<MapElementI*> DesuraJSSettings::getThemes()
{
	std::vector<ThemeStubI*> m_vThemes;
	GetGCThemeManager()->getThemeStubList(m_vThemes);

	std::vector<MapElementI*> ret;

	for (size_t x=0; x<m_vThemes.size(); x++)
	{
#ifdef WIN32
		UTIL::FS::Path file(gcString(".{0}data{0}themes", DIRS_STR), "", false);
#else
		UTIL::FS::Path file("data/themes", "", false);
#endif
		UTIL::FS::Path themePath = UTIL::FS::PathWithFile(m_vThemes[x]->getPreview());
		
		file += themePath;
		file += themePath.getFile();

		if (!UTIL::FS::isValidFile(file))
			continue;

		gcString fileUrl("desura://themeimage/{0}", m_vThemes[x]->getPreview());
		fileUrl = UTIL::STRING::sanitizeFilePath(fileUrl, '/');

		std::map<gcString, gcString> map;

		map["name"] = m_vThemes[x]->getName();
		map["displayName"] = m_vThemes[x]->getPrintName();
		map["image"] = fileUrl;
		map["tooltip"] = m_vThemes[x]->getPrintName();
		map["author"] = m_vThemes[x]->getAuthor();
		map["version"] = m_vThemes[x]->getVersion();
	
		ret.push_back(new MapElement<std::map<gcString, gcString>>(map));
	}

	safe_delete(m_vThemes);
	return ret;
}

std::vector<MapElementI*> DesuraJSSettings::getLanguages()
{
	std::vector<MapElementI*> ret;

	std::vector<UTIL::FS::Path> fileList;
	std::vector<std::string> filter;
	filter.push_back("xml");

	UTIL::FS::getAllFiles(UTIL::FS::Path(".\\data\\language", "", false), fileList, &filter);
	std::map<gcString, uint32> mSeenBefore;

	for (size_t x=0; x<fileList.size(); x++)
	{
		TiXmlDocument doc;
		doc.LoadFile(fileList[x].getFullPath().c_str());
				
		TiXmlElement *uNode = doc.FirstChildElement("lang");

		if (!uNode)
			continue;

		gcString name = uNode->ToElement()->Attribute("name");
		gcString skip = uNode->ToElement()->Attribute("skip");

		if (name.size() == 0)
			continue;

		if (skip == "true")
			continue;

		gcString file(fileList[x].getFile().getFile());
		size_t pos = file.find_last_of(".");

		if (pos != std::string::npos)
			file[pos] = '\0';

		if (mSeenBefore[name])
			name += gcString("({0})", mSeenBefore[name]);

		mSeenBefore[name]++;

		std::map<gcString, gcString> map;

		map["file"] = file;
		map["name"] = name;
	
		ret.push_back(new MapElement<std::map<gcString, gcString>>(map));
	}

	return ret;
}

std::vector<gcString> DesuraJSSettings::getSteamNames()
{
	std::vector<gcString> ret;

	gcString searchPath("{0}{1}steamapps{1}", UTIL::OS::getConfigValue(STEAMPATH), DIRS_STR);

	std::vector<UTIL::FS::Path> fileList;
	UTIL::FS::getAllFolders(UTIL::FS::Path(searchPath, "", false), fileList);

	for (size_t x=0; x<fileList.size(); x++)
	{
		if (fileList[x].getLastFolder() == "common" || fileList[x].getLastFolder() == "SourceMods" || fileList[x].getLastFolder() == "media")
			continue;

		ret.push_back(fileList[x].getLastFolder());
	}

	if (ret.size() == 0)
		ret.push_back(NOSTEAM);

	return ret;
}









std::vector<MapElementI*> DesuraJSSettings::getCurrentCIPItems()
{
	std::vector<UserCore::Misc::CIPItem> list;
	GetUserCore()->getCIPManager()->getCIPList(list);

	std::vector<MapElementI*> ret;

	for (size_t x=0; x<list.size(); x++)
	{
		std::map<gcString, gcString> map;

		map["name"] = list[x].name;
		map["path"] = list[x].path;
		map["id"] = list[x].id.toString();

		ret.push_back(new MapElement<std::map<gcString, gcString>>(map));
	}

	return ret;
}

std::vector<MapElementI*> DesuraJSSettings::getAllCIPItems()
{
	std::vector<UserCore::Misc::CIPItem> list;
	GetUserCore()->getCIPManager()->getItemList(list);

	std::vector<MapElementI*> ret;

	for (size_t x=0; x<list.size(); x++)
	{
		std::map<gcString, gcString> map;

		map["name"] = list[x].name;
		map["id"] = list[x].id.toString();

		ret.push_back(new MapElement<std::map<gcString, gcString>>(map));
	}

	return ret;
}

void DesuraJSSettings::updateCIPList()
{
	GetUserCore()->getCIPManager()->refreshList();
}

void DesuraJSSettings::saveCIPList(std::vector<std::map<gcString, gcString>> savelist)
{
	std::vector<UserCore::Misc::CIPItem> list;
	GetUserCore()->getCIPManager()->getCIPList(list);

	for (size_t x=0; x<savelist.size(); x++)
	{
		for (size_t y=0; y<list.size(); y++)
		{
			if (list[y].id.toString() == savelist[x]["id"])
			{
				list.erase(list.begin()+y);
				break;
			}
		}

		DesuraId id(UTIL::MISC::atoll(savelist[x]["id"].c_str()));

		if (id.isOk() == false)
			continue;

		GetUserCore()->getCIPManager()->updateItem(id, savelist[x]["path"]);
	}

	for (size_t x=0; x<list.size(); x++)
	{
		GetUserCore()->getCIPManager()->deleteItem(list[x].id);
	}
}

bool DesuraJSSettings::isValidCIPPath(gcString path)
{
	return UTIL::FS::isValidFolder(path);
}

gcWString DesuraJSSettings::browseCIPPath(gcWString name, gcWString path)
{
	wxDirDialog p(g_pMainApp->getMainWindow(), gcWString(L"{0} {1}", Managers::GetString(L"#CIP_BROWSE"), name), path, wxDIRP_DIR_MUST_EXIST);

	if (p.ShowModal() == wxID_OK)
		return gcWString(p.GetPath().c_str().AsWChar());

	return path;
}

bool DesuraJSSettings::isValidLinkBinary(gcString path)
{
	if (!UTIL::FS::isValidFile(path))
		return false;

#ifdef WIN32
	return (path.find_last_of(".exe") == path.size()-1);
#else
	char magicBytes[5] = {0};

	try
	{
		UTIL::FS::FileHandle fh(path.c_str(), UTIL::FS::FILE_READ);
		fh.read(magicBytes, 5);
	}
	catch (...)
	{
		return false;
	}

	return UTIL::LIN::getFileType(magicBytes, 5) != UTIL::LIN::BT_UNKNOWN;
#endif
}

gcString DesuraJSSettings::browseLinkBinary(gcString name, gcString path)
{
	wxFileDialog p(g_pMainApp->getMainWindow(), gcWString(L"{0} {1}", Managers::GetString(L"#CIP_LINK_BROWSE"), name), path);

	if (p.ShowModal() == wxID_OK)
		return gcString(p.GetPath().c_str().AsWChar());

	return path;
}
