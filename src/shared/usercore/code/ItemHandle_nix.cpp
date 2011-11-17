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
#include "ItemHandle.h"

#include "User.h"

#ifdef NIX
#include "util/UtilLinux.h"
#endif

namespace UserCore
{
namespace Item
{

void ItemHandle::doLaunch(Helper::ItemLaunchHelperI* helper)
{
	char magicBytes[5] = {0};
	UserCore::Item::Misc::ExeInfoI* ei = getItemInfo()->getActiveExe();
	
	const char* exe = ei->getExe();
	const char* args = getUserCore()->getCVarValue("gc_linux_launch_globalargs");
	gcString globalExe = getUserCore()->getCVarValue("gc_linux_launch_globalbin");
	
	if (globalExe.size() > 0)
	{
		if (!UTIL::FS::isValidFile(globalExe.c_str()))
		{
			Warning(gcString("Couldnt find global exe [{0}], ignoring.\n", globalExe));
			globalExe = "";
			exe = ei->getExe();
		}
		else
		{
			exe = globalExe.c_str();
		}
	}
	
	try
	{
		UTIL::FS::FileHandle fh(exe, UTIL::FS::FILE_READ);
		fh.read(magicBytes, 5);
	}
	catch (gcException& e)
	{
		throw gcException(ERR_LAUNCH, e.getSecErrId(), gcString("Failed to read [{0}]: {1}\n", exe, e));
	}

	UTIL::LIN::BinType type = UTIL::LIN::getFileType(magicBytes, 5);

	if (type == UTIL::LIN::BT_WIN && helper)
		helper->showWinLaunchDialog();
	
	//if we are not using globalExe set exe to Null so that we use the proper exe
	if (globalExe.size() == 0 || type == UTIL::LIN::BT_UNKNOWN)
		exe = NULL;

	doLaunch(type == UTIL::LIN::BT_UNKNOWN, exe, args);
}

void ItemHandle::doLaunch(bool useXdgOpen, const char* globalExe, const char* globalArgs)
{
	preLaunchCheck();
	
	UserCore::Item::Misc::ExeInfoI* ei = getItemInfo()->getActiveExe();
	
	gcString e(globalExe);
	
	gcString args;
	gcString ea(ei->getExeArgs());
	gcString ua(ei->getUserArgs());
	gcString ga(globalArgs);
	gcString wdp(ei->getExe());
	
	if (!useXdgOpen)
	{
		//if we have a valid global exe need to append the local exe as the first arg
		if (e.size() > 0)
			args += gcString(ei->getExe());
		else
			globalExe = ei->getExe();
	}
	
	auto AppendArgs = [&args](const std::string& a)
	{
		if (a.size() == 0)
			return;
		
		if (args.size() > 0)
			args += " ";
			
		args += a;
	};

	AppendArgs(ea);
	AppendArgs(ua);
	
	if (!useXdgOpen)
		AppendArgs(ga);
	
	bool res = false;

	if (useXdgOpen && args.size() != 0)
		Warning(gcString("Arguments '{1}' are not being passed to non-executable file '{0}'.", ei->getExe(), args));

	UserCore::Item::BranchInfoI* branch = getItemInfo()->getCurrentBranch();

#ifdef NIX64
	if (!useXdgOpen && branch && branch->is32Bit())
	{
		int testRet = system("bin/bittest");
		
		if (testRet != 0)
			throw gcException(ERR_NO32LIBS);
	}
#endif

	gcString libPathA;
	gcString libPathB;
	gcString libPath;
	
	if (branch)
	{
		libPathA = gcString("{0}/{1}/{2}/lib", UTIL::OS::getAppDataPath(), branch->getItemId().getFolderPathExtension(), (uint32)branch->getBranchId());
		libPathB = gcString("{0}/lib{1}", getItemInfo()->getPath(), branch->is32Bit()?"32":"64");

		libPath = libPathA;
		
		if (UTIL::FS::isValidFolder(libPathB.c_str()))
		{
			libPath += ":";
			libPath += libPathB;
		}
	}

	if (useXdgOpen)
	{
		res = UTIL::LIN::launchProcessXDG(ei->getExe(), libPath.c_str());
	}
	else
	{
		if (libPathA.size() > 0)
		{
			std::vector<DesuraId> toolList;
			branch->getToolList(toolList);
			
			getUserCore()->getToolManager()->symLinkTools(toolList, libPathA.c_str());
		}
	
		std::map<std::string, std::string> info;
		
		info["cla"] = args;
		info["lp"] = libPath;
		info["wd"] = UTIL::FS::PathWithFile(wdp.c_str()).getFolderPath();
		
		res = UTIL::LIN::launchProcess(globalExe, info);
	}

	if (!res)
	{
		ERROR_OUTPUT(gcString("Failed to create {0} process. [{1}: {2}].\n", getItemInfo()->getName(), errno, ei->getExe()).c_str());
		throw gcException(ERR_LAUNCH, errno, gcString("Failed to create {0} process. [{1}: {2}].\n", getItemInfo()->getName(), errno, ei->getExe()));
	}
}

void ItemHandle::installLaunchScripts()
{
	UserCore::Item::ItemInfoI* item = getItemInfo();
	
	if (!item)
		return;
		
	UserCore::Item::BranchInfoI* branch = item->getCurrentBranch();
	
	if (!branch)
		return;
		
	std::vector<UserCore::Item::Misc::ExeInfoI*> exeList;
	item->getExeList(exeList);
	
	char* scriptBin = NULL;
	char* scriptXdg = NULL;
	
	try
	{
		UTIL::FS::readWholeFile("./data/scripts/launch_bin_template.sh", &scriptBin);
		UTIL::FS::readWholeFile("./data/scripts/launch_xdg_template.sh", &scriptXdg);
	}
	catch (gcException &e)
	{
		safe_delete(scriptBin);
		safe_delete(scriptXdg);		
		
		Warning(gcString("Failed to read launch script template: {0}\n", e));
		return;
	}
	
	gcString globalArgs = getUserCore()->getCVarValue("gc_linux_launch_globalargs");
	gcString globalExe = getUserCore()->getCVarValue("gc_linux_launch_globalbin");
	
	if (!UTIL::FS::isValidFile(globalExe.c_str()))
		globalExe = "";
	
	for (size_t x=0; x<exeList.size(); x++)
	{
		UserCore::Item::Misc::ExeInfoI* exe = exeList[x];
		
		if (!exe || !UTIL::FS::isValidFile(exe->getExe()))
			continue;
			
		gcString path("{0}/desura_launch_{1}.sh", item->getPath(), UTIL::LIN::sanitiseFileName(exe->getName()));
			
		char magicBytes[5] = {0};
			
		try
		{
			UTIL::FS::FileHandle fh(exe->getExe(), UTIL::FS::FILE_READ);
			fh.read(magicBytes, 5);
		}
		catch (gcException& e)
		{
			continue;
		}
		
		UTIL::LIN::BinType type = UTIL::LIN::getFileType(magicBytes, 5);
		
		try
		{
			UTIL::FS::FileHandle fh(path.c_str(), UTIL::FS::FILE_WRITE);
			
			if (type == UTIL::LIN::BT_UNKNOWN)
			{
				gcString lcmd(scriptXdg, exe->getExe());
				fh.write(lcmd.c_str(), lcmd.size());
			}
			else
			{
				gcString libPath("\"{0}/{1}/{2}/lib\"", UTIL::OS::getAppDataPath(), branch->getItemId().getFolderPathExtension(), (uint32)branch->getBranchId());
				gcString libPathB("{0}/lib{1}", item->getPath(), branch->is32Bit()?"32":"64");
				
				if (UTIL::FS::isValidFolder(libPathB.c_str()))
				{
					libPath += ":";
					libPath += "\"" + libPathB + "\"";
				}

				const char* exePath = exe->getExe();
				
				gcString args;
				gcString ea(exe->getExeArgs());
				
				if (globalExe.size() > 0)
				{
					args += gcString(exePath);
					exePath = globalExe.c_str();
				}
				
				if (ea.size() > 0)
				{
					if (args.size() > 0)
						args += " ";
						
					args += ea;
				}
				
				if (globalArgs.size() > 0)
				{
					if (args.size() > 0)
						args += " ";
						
					args += globalArgs;
				}			
					
				gcString lcmd(scriptBin, exePath, args, libPath);
				fh.write(lcmd.c_str(), lcmd.size());
			}
		}
		catch (gcException &e)
		{
		}
		
		chmod(path.c_str(), S_IRWXU|S_IRGRP|S_IROTH);
	}
	
	safe_delete(scriptBin);
	safe_delete(scriptXdg);
}


}
}
