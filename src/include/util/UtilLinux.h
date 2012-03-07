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

#ifndef UTIL_LIN_H
#define UTIL_LIN_H
#ifdef _WIN32
#pragma once
#endif

#ifdef NIX

#include "util/UtilString.h"

namespace UTIL
{
namespace LIN
{
	enum BinType
	{
		BT_ELF32,	// Warn and launch natively if on x64
		BT_ELF64,	// Launch natively
		BT_WIN,		// Warn and launch using xdg-open
		BT_SCRIPT,	// Launch using xdg-open
		BT_UNKNOWN,	// Error
	};
	
	//! Returns a std::string of the path expanded
	//!
	//! @param file string to file to expand
	//! @return expanded path if successful, "" if not
	//!
	std::string expandPath(const char* file);

	//! Returns the type of file by its magic mark at the start of the file
	//!
	//! @param buff Buffer that contains at least the first 4 bytes
	//! @param buffSize size of buffer
	//! @return Bin type if known or BT_UNKNOWN if not
	//!
	BinType getFileType(const char* buff, size_t buffSize);
	
	//! Gets the exectuables directory
	//!
	//! @param optional extra string to append to path
	//! @return exectuables directory
	//!
	std::wstring getAppPath(std::wstring extra);

	//! Gets the exectuables data directory
	//!
	//! @param optional extra string to append to path
	//! @return exectuables data directory
	//!
	std::wstring getAppDataPath(std::wstring extra);

	//! Gets an OS information string
	//!
	//! @return OS information stirng
	//!
	std::string getOSString();

	//! Sets a config key
	//!
	//! @param configKey Key to set
	//! @param value Value to set it to
	//!
	void setConfigValue(const std::string &configKey, const std::string &value);
	void setConfigValue(const std::string &configKey, uint32 value);

	//! Gets a config value based on a key
	//!
	//! @param config key to lookup
	//! @return config key value
	//!
	std::string getConfigValue(const std::string &configKey);
	
	//! Gets the ammount of free space on a drive
	//!
	//! @param path Drive to analize
	//! @return Free space in bytes
	//!
	uint64 getFreeSpace(const char* path);

	//! Launches a process by forking
	//!
	//! @param exe Process to launch (binary elf or shell script)
	//! @param info Map of extra information:
	//!		"cla" 	Command line arguments
	//!		"lp" 	LD_LIBRARY_PATH
	//!		"wd"	Working dir (or uses the exe root dir by default)
	//! @return true if process was launched
	//!
	bool launchProcess(const char* exe, const std::map<std::string, std::string> &info = std::map<std::string, std::string>());

	// Launch a process in $PATH
	bool launchProcessXDG(const char* exe, const char* libPath = NULL);

	bool launchFolder(const char* path);

	//! Returns the stdout of a system() call
	//!
	//! @param command command to execute
	//! @param stdErrDest 0 = do nothing, 1 = hide it, 2 = append to stdout
	//! @return stdout
	//! @note Throws exceptions if can't run command
	//!
	std::string getCmdStdout(const char* command, int stdErrDest = 0);
	
	//! Returns the Desktop directory in the
	//! user's home.
	//!
	//! @param extra extra text to append to Desktop path
	//!
	std::wstring getDesktopPath(std::wstring extra);

	//! Checks if a file exists
	//!
	//! @param file The file to stat (check existance of)
	//! @return if file exists
	//!
	bool fileExists(const char* file);
	
	bool is64OS();

	gcString getAbsPath(const gcString& path);
	gcString getRelativePath(const gcString &path);
	
	std::string sanitiseFileName(const char* name);
	
	//! Sets up XDG environmental variables.
	//!
	//! @return True if an error occured.
	//!
	bool setupXDGVars();
	
	//! Updates the $XDG_RUNTIME_DIR/desura file timestamps.
	void updateXDGRuntimeStamps();
}
}

#endif

#endif
