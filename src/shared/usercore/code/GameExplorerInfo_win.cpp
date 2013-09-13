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
#include "GameExplorerManager.h"

#include "ItemInfo.h"
#include "usercore\UserCoreI.h"
#include "usercore\ItemManagerI.h"

#include "IPCServiceMain.h"

#include "XMLMacros.h"

#include "sqlite3x.hpp"

#pragma pack(push)
#pragma pack(2)

typedef struct 
{
	BYTE bWidth;		// Width, in pixels, of the image
	BYTE bHeight;		// Height, in pixels, of the image
	BYTE bColorCount;	// Number of colors in image (0 if >=8bpp)
	BYTE bReserved;		// Reserved
	WORD wPlanes;		// Color Planes
	WORD wBitCount;		// Bits per pixel
	DWORD dwBytesInRes; // how many bytes in this resource?
	DWORD dwImageOffeset; // the ID
} 
ICONDIRENTRY;

typedef struct 
{
	WORD idReserved;	// Reserved (must be 0)
	WORD idType;		// Resource type (1 for icons)
	WORD idCount;		// How many images?
	ICONDIRENTRY idEntries[1]; // The entries for each image
} 
ICONDIR;

typedef struct 
{
	BYTE bWidth;		// Width, in pixels, of the image
	BYTE bHeight;		// Height, in pixels, of the image
	BYTE bColorCount;	// Number of colors in image (0 if >=8bpp)
	BYTE bReserved;		// Reserved
	WORD wPlanes;		// Color Planes
	WORD wBitCount;		// Bits per pixel
	DWORD dwBytesInRes; // how many bytes in this resource?
	WORD nID;			// the ID
} 
GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

typedef struct 
{
	WORD idReserved;	// Reserved (must be 0)
	WORD idType;		// Resource type (1 for icons)
	WORD idCount;		// How many images?
	GRPICONDIRENTRY idEntries[1]; // The entries for each image
} 
GRPICONDIR, *LPGRPICONDIR;


#pragma pack(pop)


tinyxml2::XMLElement* WriteChildWithChildAndAtt(const char* nodeName, const char* childName, const char* attName, const char* attValue, tinyxml2::XMLNode* root, tinyxml2::XMLDocument& doc)
{
	tinyxml2::XMLElement *node = doc.NewElement(nodeName);
	tinyxml2::XMLElement *child = doc.NewElement(childName);

	child->SetAttribute(attName, attValue);

	node->LinkEndChild(child);
	root->LinkEndChild(node);

	return node;
}

tinyxml2::XMLElement* WriteChildWithChildAndAtt(tinyxml2::XMLElement* node, const char* childName, const char* attName, const char* attValue, tinyxml2::XMLDocument& doc)
{
	tinyxml2::XMLElement *child = doc.NewElement(childName);
	child->SetAttribute(attName, attValue);
	return node;
}


namespace UserCore
{
namespace Misc
{

GameExplorerInfo::GameExplorerInfo(DesuraId id, UserCore::UserI* user)
{
	m_Id = id;

	m_uiHash = id.toInt64();
	m_pUser = user;
	m_uiFlags = 0;

	m_pItemInfo = NULL;

	regEvent();
}

GameExplorerInfo::~GameExplorerInfo()
{
	if (m_pItemInfo)
		*m_pItemInfo->getInfoChangeEvent() -= delegate(this, &GameExplorerInfo::onInfoChanged);
}

void GameExplorerInfo::regEvent()
{
	if (m_pItemInfo)
		return;

	m_pItemInfo = m_pUser->getItemManager()->findItemInfo(m_Id);

	if (!m_pItemInfo)
		return;

	Event<UserCore::Item::ItemInfoI::ItemInfo_s> &e = *m_pItemInfo->getInfoChangeEvent();
	e += delegate(this, &GameExplorerInfo::onInfoChanged);
}

void GameExplorerInfo::onInfoChanged(UserCore::Item::ItemInfoI::ItemInfo_s &info)
{
	if (!HasAnyFlags(info.changeFlags, UserCore::Item::ItemInfoI::CHANGED_INFO|UserCore::Item::ItemInfoI::CHANGED_LOGO|UserCore::Item::ItemInfoI::CHANGED_ICON))
		return;

	if (!UTIL::FS::isValidFile(m_pItemInfo->getIcon()) || !UTIL::FS::isValidFile(m_pItemInfo->getLogo()))
		return;

	m_uiFlags |= FLAG_NEEDSUPATE;
}

void GameExplorerInfo::loadFromDb(sqlite3x::sqlite3_connection *db)
{
	if (!db)
		return;

	sqlite3x::sqlite3_command cmd(*db, "SELECT dllpath, guid, flags FROM gameexplorer WHERE internalid=?;");
	cmd.bind(1, (int64)m_Id.toInt64());

	sqlite3x::sqlite3_reader reader = cmd.executereader();
	reader.read();

	m_szDllPath		= reader.getstring(0);
	m_szGuid		= reader.getstring(1);
	m_uiFlags		= reader.getint(2);

	if (!UTIL::FS::isValidFile(m_szDllPath))
	{
		m_szDllPath = "";
		m_uiFlags &= ~FLAG_INSTALLED;
	}
}

void GameExplorerInfo::saveToDb(sqlite3x::sqlite3_connection *db)
{
	if (!db)
		return;

	sqlite3x::sqlite3_command cmd(*db, "REPLACE INTO gameexplorer VALUES (?,?,?,?);");

	cmd.bind(1, (int64)m_Id.toInt64());
	cmd.bind(2, m_szDllPath);				//name
	cmd.bind(3, m_szGuid);		//flags
	cmd.bind(4, (int)m_uiFlags);

	cmd.executenonquery();
}

bool GameExplorerInfo::isInstalled()
{
	return m_uiFlags & FLAG_INSTALLED;
}

bool GameExplorerInfo::needsUpdate()
{
	return m_uiFlags & FLAG_NEEDSUPATE;
}

bool GameExplorerInfo::isInstallable()
{
	return m_pItemInfo && HasAllFlags(m_pItemInfo->getStatus(), UserCore::Item::ItemInfoI::STATUS_ONCOMPUTER) && m_pItemInfo->isDownloadable();
}

bool GameExplorerInfo::needsInstall()
{
	return  (!isInstalled() || needsUpdate());
}

void GameExplorerInfo::installDll()
{
	if (!UTIL::FS::isValidFile(m_szDllPath))
		return;

	if (!m_pItemInfo)
		return;

	if (m_pUser->getServiceMain())
		m_pUser->getServiceMain()->addItemGameToGameExplorer(m_pItemInfo->getName(), m_szDllPath.c_str());

	m_uiFlags |= FLAG_INSTALLED;
	m_uiFlags &= ~FLAG_NEEDSUPATE;
}

void GameExplorerInfo::removeDll()
{
	if (!UTIL::FS::isValidFile(m_szDllPath))
		return;

	if (m_pUser->getServiceMain())
		m_pUser->getServiceMain()->removeGameFromGameExplorer(m_szDllPath.c_str(), true);

	m_szDllPath = "";

	m_uiFlags = 0;
}

void GameExplorerInfo::generateDll()
{
	regEvent();

	if (!m_pItemInfo)
		return;

	if (!UTIL::FS::isValidFile(m_pItemInfo->getIcon()) || !UTIL::FS::isValidFile(m_pItemInfo->getLogo()))
		return;

	std::string appDataPath = m_pUser->getAppDataPath();

	if (!UTIL::FS::isValidFile(m_szDllPath))
	{
		m_szDllPath = gcString("{0}\\GDF\\{1}.dll", appDataPath, m_Id.toInt64());

		UTIL::FS::Path folderPath(m_szDllPath, "", true);

		if (!UTIL::FS::isValidFolder(folderPath))
			UTIL::FS::recMakeFolder(folderPath);
	}

	UTIL::FS::delFile(m_szDllPath);
	UTIL::FS::delFile(m_szDllPath + "_old");

	if (UTIL::FS::isValidFile(m_szDllPath))
		UTIL::FS::moveFile(m_szDllPath, m_szDllPath + "_old");

	UTIL::FS::copyFile(".\\bin\\Blank_GDF.dll", m_szDllPath);

	const char* logoPath = m_pItemInfo->getLogo();
	const char* iconPath = m_pItemInfo->getIcon();

	gcString tempIco("{0}\\temp\\{1}.ico", appDataPath, m_Id.toInt64());
	gcString tempPng("{0}\\temp\\{1}.png", appDataPath, m_Id.toInt64());

	UTIL::FS::recMakeFolder(UTIL::FS::PathWithFile(tempIco));

	gcWString xmlData = generateXml();
	
	if (!UTIL::MISC::convertToIco(iconPath, tempIco))
	{
		int a=1;
	}

	if (!UTIL::MISC::convertToPng(logoPath, tempPng, 256))
	{
		int a=1;
	}

	HANDLE handle = BeginUpdateResource(m_szDllPath.c_str(), true);

	if (!handle)
		return;

	char *logoData = NULL;
	uint32 logoSize = 0;

	try
	{
		logoSize = UTIL::FS::readWholeFile(tempPng, &logoData);
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to read icon file: {0}\n", e));

		safe_delete(logoData);
		EndUpdateResource(handle, FALSE);

		UTIL::FS::delFile(m_szDllPath.c_str());
		return;
	}

	UpdateResource(handle, "DATA", "__GDF_XML", MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPVOID)xmlData.c_str(), xmlData.size()*2);	//xml (size times two cause we are treating it as char*)
	UpdateResource(handle, "DATA", "__GDF_THUMBNAIL", MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPVOID)logoData, logoSize);	//logo
	translateIco(handle, tempIco.c_str());

	safe_delete(logoData);
	EndUpdateResource(handle, FALSE);

	UTIL::FS::delFile(tempIco);
	UTIL::FS::delFile(tempPng);
}

void GameExplorerInfo::translateIco(HANDLE handle, const char* icoPath)
{
	char* buff = NULL;
	uint32 size = UTIL::FS::readWholeFile(icoPath, &buff);


	// Change the icon group
	ICONDIR* ig = (ICONDIR*)(buff);

	DWORD newSize = sizeof(GRPICONDIR) + (sizeof(GRPICONDIRENTRY)*(ig->idCount - 1));

	UTIL::MISC::Buffer outBuff(newSize, true);

	GRPICONDIR* newDir = (GRPICONDIR*)outBuff.data();
	newDir->idReserved	= ig->idReserved;
	newDir->idType		= ig->idType;
	newDir->idCount		= ig->idCount;

	const char* type = RT_ICON;
	const char* name;
	DWORD rv;

	for (size_t i=0; i<ig->idCount; i++)
	{
		BYTE* temp1 = (BYTE*)buff + ig->idEntries[i].dwImageOffeset;
		DWORD size1 = ig->idEntries[i].dwBytesInRes;

		newDir->idEntries[i].bWidth = ig->idEntries[i].bWidth;
		newDir->idEntries[i].bHeight = ig->idEntries[i].bHeight;
		newDir->idEntries[i].bColorCount = ig->idEntries[i].bColorCount;
		newDir->idEntries[i].bReserved = ig->idEntries[i].bReserved;
		newDir->idEntries[i].wPlanes = ig->idEntries[i].wPlanes;
		newDir->idEntries[i].wBitCount = ig->idEntries[i].wBitCount;
		newDir->idEntries[i].dwBytesInRes = ig->idEntries[i].dwBytesInRes;
		newDir->idEntries[i].nID = i + 1;

		name = MAKEINTRESOURCE(i + 1);
		rv = UpdateResource(handle, type, name, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (PVOID)temp1, size1);
	}

	type = RT_GROUP_ICON;
	name = MAKEINTRESOURCE(101);

	rv = UpdateResource(handle, type, name, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (PVOID)newDir, newSize);
	safe_delete(buff);
}

gcString GameExplorerInfo::generateGuid()
{
	GUID guid;
	HRESULT res = CoCreateGuid(&guid);

	wchar_t out[200] = {0};
	StringFromGUID2(guid, out, 200);

	return gcString(gcWString(out));
}

gcWString GameExplorerInfo::generateXml()
{
	gcString szName = m_pItemInfo->getName();
	gcString szVersion("1.0.0.0"); //{0}.{1}", (int)m_pItemInfo->getInstalledBranch(), (int)m_pItemInfo->getInstalledBuild());
	gcString szDevUrl = m_pItemInfo->getDevProfile();
	gcString szDevName = m_pItemInfo->getDev();

	gcString szPub = m_pItemInfo->getPublisher();
	gcString szPubUrl = m_pItemInfo->getPublisherProfile();

	if (szPub.size() == 0)
		szPub = "Unknown";

	if (szPubUrl.size() == 0)
		szPubUrl = "http://";

	gcString szPlayLink("desura://launch/{0}/{1}", m_Id.getTypeString(), m_pItemInfo->getShortName());
	gcString szChangeLogLink("http://www.desura.com/{0}/{1}/changelog", m_Id.getTypeString(), m_pItemInfo->getShortName());
	gcString szProfileLink = m_pItemInfo->getProfile();

	gcString szDescription = m_pItemInfo->getName();
	gcString szReleaseData = "2010-01-01";

	gcString szUninstallLink("desura://uninstall/{0}/{1}", m_Id.getTypeString(), m_pItemInfo->getShortName());
	gcString szVerifyLink("desura://verify/{0}/{1}", m_Id.getTypeString(), m_pItemInfo->getShortName());

	gcString szGenere(m_pItemInfo->getGenre());
	
	std::vector<UserCore::Item::Misc::ExeInfoI*> vExeList;
	m_pItemInfo->getExeList(vExeList);


	if (m_szGuid == "")
		m_szGuid = generateGuid();

	if (szGenere.size() == 0)
		szGenere = "Unknown";

	tinyxml2::XMLDocument doc;

	tinyxml2::XMLDeclaration *decl = doc.NewDeclaration("<?xml version=\"1.0\" encoding=\"UTF-16\" ?>");
	tinyxml2::XMLElement *gameDefFile = doc.NewElement("GameDefinitionFile");
	tinyxml2::XMLElement *gameDef = doc.NewElement("GameDefinition");

	gameDefFile->SetAttribute("xmlns:baseTypes", "urn:schemas-microsoft-com:GamesExplorerBaseTypes.v1");
	gameDefFile->SetAttribute("xmlns", "urn:schemas-microsoft-com:GameDescription.v1");
	gameDef->SetAttribute("gameID", m_szGuid.c_str());


	XML::WriteChild("Name", szName, gameDef, doc);
	XML::WriteChild("Description", szDescription, gameDef, doc);
	XML::WriteChild("ReleaseDate", szReleaseData, gameDef, doc);

	tinyxml2::XMLElement *genres = doc.NewElement("Genres");
	XML::WriteChild("Genre", szGenere, genres, doc);
	gameDef->InsertEndChild(genres);
	
	WriteChildWithChildAndAtt("Version", "VersionNumber", "versionNumber", szVersion.c_str(), gameDef, doc);


	tinyxml2::XMLElement *devs = doc.NewElement("Developers");
	tinyxml2::XMLElement *dev = doc.NewElement("Developer");

			dev->SetAttribute("URI", szDevUrl.c_str());
			dev->InsertEndChild(doc.NewText(szDevName.c_str()));

		devs->InsertEndChild(dev);
	gameDef->InsertEndChild(devs);


	tinyxml2::XMLElement *publishers = doc.NewElement("Publishers");
	tinyxml2::XMLElement *publisher = doc.NewElement("Publisher");

			publisher->SetAttribute("URI", szPubUrl.c_str());
			publisher->InsertEndChild(doc.NewText(szPub.c_str()));

		publishers->InsertEndChild(publisher);
	gameDef->InsertEndChild(publishers);


	
	if (vExeList.size() > 0)
	{
		tinyxml2::XMLElement *gameExecutables = doc.NewElement("GameExecutables");

		for (size_t x=0; x<vExeList.size(); x++)
		{
			gcString szGameExe(UTIL::FS::PathWithFile(vExeList[x]->getExe()).getFile().getFile());
			tinyxml2::XMLElement *gameExecutable = doc.NewElement("GameExecutable");

			gameExecutable->SetAttribute("path", szGameExe.c_str());
			gameExecutables->InsertEndChild(gameExecutable);
		}

		gameDef->InsertEndChild(gameExecutables);
	}

	int i = 1;

	tinyxml2::XMLElement *extProps = doc.NewElement("ExtendedProperties");
	tinyxml2::XMLElement *gameTasks = doc.NewElement("GameTasks");

	tinyxml2::XMLElement *playTask = doc.NewElement("Play");

		WriteChildWithChildAndAtt("Primary", "URLTask", "Link", szPlayLink.c_str(), playTask, doc);

		if (vExeList.size() > 1)
		{
			for (size_t x=0; x<vExeList.size(); x++)
			{
				gcString play("Play: {0}", vExeList[x]->getName());
				gcString link("desura://launch/{0}/{1}/{2}", m_Id.getTypeString(), m_pItemInfo->getShortName(), vExeList[x]->getName());

				tinyxml2::XMLElement *changeLog = WriteChildWithChildAndAtt("Task", "URLTask", "Link", link.c_str(), playTask, doc);
				changeLog->SetAttribute("index", i);
				changeLog->SetAttribute("name", play.c_str());
				i++;
			}
		}

		if (m_pItemInfo->isDownloadable())
		{
			tinyxml2::XMLElement *changeLog = WriteChildWithChildAndAtt("Task", "URLTask", "Link", szChangeLogLink.c_str(), playTask, doc);
			changeLog->SetAttribute("index", i);
			changeLog->SetAttribute("name", "View Update History");
			i++;
		}
		
		
		tinyxml2::XMLElement *profile = WriteChildWithChildAndAtt("Task", "URLTask", "Link", szProfileLink.c_str(), playTask, doc);
		profile->SetAttribute("index", i);
		profile->SetAttribute("name", "View Profile");
		i++;
		

		uint32 count = 0;
		for (uint32 x=0; x<m_pItemInfo->getBranchCount(); x++)
		{
			UserCore::Item::BranchInfoI* bi = m_pItemInfo->getBranch(x);

			if (!bi)
				continue;

			if (bi->getBranchId() == m_pItemInfo->getInstalledBranch())
				continue;

			if (bi->getFlags()&UserCore::Item::BranchInfoI::BF_NORELEASES)
				continue;

			if (!(bi->getFlags()&UserCore::Item::BranchInfoI::BF_ONACCOUNT) && !(bi->getFlags()&UserCore::Item::BranchInfoI::BF_FREE))
				continue;

			gcString name("Install Branch: {0}", bi->getName());
			gcString link("desura://install/{0}/{1}/{2}", m_Id.getTypeString(), m_pItemInfo->getShortName(), bi->getBranchId());

			tinyxml2::XMLElement *branch = WriteChildWithChildAndAtt("Task", "URLTask", "Link", link.c_str(), playTask, doc);
			branch->SetAttribute("index", x+i);
			branch->SetAttribute("name", name.c_str());
			
		}


	gameTasks->LinkEndChild(playTask);

	tinyxml2::XMLElement *supportTask = doc.NewElement("Support");

	tinyxml2::XMLElement *verify = WriteChildWithChildAndAtt("Task", "URLTask", "Link", szVerifyLink.c_str(), supportTask, doc);
		verify->SetAttribute("index", 0);
		verify->SetAttribute("name", "Verify Files");

		tinyxml2::XMLElement *uninstall = WriteChildWithChildAndAtt("Task", "URLTask", "Link", szUninstallLink.c_str(), supportTask, doc);
		uninstall->SetAttribute("index", 1);
		uninstall->SetAttribute("name", "Uninstall");
		

	gameTasks->LinkEndChild(supportTask);
	extProps->LinkEndChild(gameTasks);
	gameDef->LinkEndChild(extProps);
	gameDefFile->LinkEndChild(gameDef);

	doc.LinkEndChild(decl);
	doc.LinkEndChild(gameDefFile);

	tinyxml2::XMLPrinter printer;
	doc.Accept( &printer );

	gcWString res;

	res.resize(1);

	res[0] = 0xFF + (0xFE<<8);
	res += gcWString(printer.CStr());

	return res;
}


}
}
