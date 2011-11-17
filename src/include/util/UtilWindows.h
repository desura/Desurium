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

#ifndef DESURA_WIN_H
#define DESURA_WIN_H


enum OS_VERSION
{
	WINDOWS_UNKNOWN,
	WINDOWS_SERVER2008,
	WINDOWS_SERVER2003,
	WINDOWS_SERVER2000,
	WINDOWS_HOMESERVER,
	WINDOWS_VISTA,
	WINDOWS_XP64,
	WINDOWS_XP,
	WINDOWS_2000,
	WINDOWS_PRE2000,
	WINDOWS_7,
};

enum SERVICE_STATUS_E
{
	SERVICE_STATUS_NOTINSTALLED,
	SERVICE_STATUS_CONTINUE_PENDING,
	SERVICE_STATUS_PAUSE_PENDING,
	SERVICE_STATUS_PAUSED,
	SERVICE_STATUS_START_PENDING,
	SERVICE_STATUS_RUNNING,
	SERVICE_STATUS_STOP_PENDING,
	SERVICE_STATUS_STOPPED,
	SERVICE_STATUS_UNKNOWN,
};

enum SC_PROPERTY
{
	EXCLUDE_NEW_INSTALL,
	EXCLUDE_PIN_STARTMENU,
};

namespace UTIL
{
namespace WIN
{
	//! Checks to see if windows is running in 64 bit mode
	//!
	//! @return Ture if running 64bit, false if not
	//!
	bool is64OS();

	//! Gets a registry key
	//!
	//! @param regIndex Key to get
	//! @param dest Buffer to put response into
	//! @param destSize Size of dest buffer
	//! @param use64bit Look in the 64 bit registy instead of the default 32
	//!
	std::string getRegValue(const std::string &regIndex, bool use64bit = false);
	int getRegValueInt(const std::string &regIndex, bool use64bit = false);

	//! Sets a registry key
	//!
	//! @param regIndex Key to set
	//! @param value Value to set it to
	//! @param type Type of reg key, default is zero = string  
	//!
	void setRegValue(const std::string &regIndex, const std::string &value, bool expandStr = false, bool use64bit = false);
	void setRegValue(const std::string &regIndex, uint32 value, bool use64bit = false);
	void setRegBinaryValue(const std::string &regIndex, const char* blob, size_t size, bool use64bit);

	//! Deletes a registry value
	//!
	//! @param regIndex Value to delete
	//! @return True if deleted
	//!
	bool delRegValue(const std::string &regIndex, bool use64bit = false);

	//! Deletes a registry key
	//!
	//! @param regIndex Key to delete
	//! @return True if deleted
	//!
	bool delRegKey(const std::string &regIndex, bool use64bit = false);

	//! Delets a registry tree including all sub tress
	//!
	//! @parma regIndex Tree to delete
	//! @return True if deleted
	//!
	bool delRegTree(const std::string &regIndex, bool use64bit = false);

	//! Gets all keys under a reg index
	//!
	//! @parma regIndex Key to iterate
	//! @param regKeys Keys found
	//!
	void getAllRegKeys(const std::string &regIndex, std::vector<std::string> &regKeys, bool use64bit = false);

	//! Gets all values under a reg index
	//!
	//! @parma regIndex Key to iterate
	//! @param regValues Names of values found
	//!
	void getAllRegValues(const std::string &regIndex, std::vector<std::string> &regValues, bool use64bit = false);

	//! Gets the ammount of free space on a drive
	//!
	//! @param path Drive to analize
	//! @return Free space in bytes
	//!
	uint64 getFreeSpace(const char* path);

	//! Get os version string
	//!
	//! @param dest Buffer to put response into
	//! @param destSize Size of dest buffer
	//!
	void getOSString(char* dest, size_t destSize);

	//! Gets the os version id
	//!
	//! @return Os version id
	//!
	OS_VERSION getOSId();

	//! Gets primary hard drive serial
	//!
	//! @return Serial
	//!
	uint32 getHDDSerial();

	//! Renables a service
	//! 
	//! @param name Service name
	//!
	void enableService(const char* name);

	//! Starts a service
	//!
	//! @param name Serivce name
	//!
	void startService(const char* name, std::vector<std::string> &args);

	//! Stops a service
	//!
	//! @param name Serivce name
	//!
	void stopService(const char* name);

	//! Installs a service
	//!
	//! @param name Serivce name
	//! @param path Path to service exe
	//! @param dispname Display name
	//!
	void installService(const char* name, const char* path, const char* dispname);

	//! Removes a service
	//!
	//! @param name Serivce name
	//!
	void uninstallService(const char* name);

	//! Get service status
	//!
	//! @param name Serivce name
	//! @return Service status
	//!
	SERVICE_STATUS_E queryService(const char* name);

	//! Changes a service permissions so normal users can start it with out admin prompt
	//!
	//! @param name Serivce name
	//!
	void changeServiceAccess(const char* name);

	//! Validates a certificate on a file
	//!
	//! @param file to Validate
	//! @param message Error message buffer
	//! @param size Error message buffer size
	//! @return Cert status
	//!
	uint32 validateCert(const wchar_t* file, char* message = NULL, size_t size = 0);

	//! Checks to see if a point is on visable screen
	//!
	//! @param x X coord
	//! @param y Y coord
	//!
	bool isPointOnScreen(int32 x, int32 y);

	//! Is the version of dot net installed on the computer
	//!
	//! @param major Major version number 
	//! @param minor Minor version number
	//! @return true if installed, false if not
	//!
	bool isDotNetVersionInstalled(uint32 major, uint32 minor = 0, bool clientProfile = false);

	//! Checks a msi guid and see if its installed on the system
	//!
	//! @param productCode Msi code
	//! @return True if installed
	//!
	bool isMsiInstalled(const char* productCode);

	//! Creates a short cut in the file system
	//!
	//! @param path Path to save the short cut to including name and ext
	//! @param exe Path to exe
	//! @param workingDir Directory to start program in
	//! @param args Arguments to launch exe with
	//! @param flagAsNonPinned Mark the shortcut as non pinnable in the start menu
	//! @param icon Path to icon, default is the first exe icon
	//!
	void createShortCut(const wchar_t *path, const char* exe, const char* workingDir, const char* args, bool flagAsNonPinned = false, const char* icon = NULL);

	//! Starts an exe 
	//!
	//! @param exe Path to exe
	//! @param args Args to use
	//! @param winElevateIfNeeded Try evelating this exe to admin if needed
	//!
	//! Will throw exceptions
	//!
	bool launchExe(const char* exePath, const char* args=NULL, bool elevateIfNeeded = false, HWND elevationHandle = (HWND)0);

	//! Changes a folder to be writable by all users
	//!
	void changeFolderPermissions(const std::wstring& dir);

	//! Sets the firewall to allow an exe
	//!
	bool setFirewallAllow(const char* exePath, const char* name);

	//! Removes the firewall rule to allow an exe
	//!
	void delFirewallAllow(const char* exePath);

	//! Gets the path to the windows install folder
	//!
	gcWString getWindowsPath(const wchar_t *extra = NULL);

	template <typename T>
	void extractIcon(const char* exe, const T &callback)
	{
		UTIL::CB::TemplateCallback<const T> c(callback);
		extractIconCB(exe, &c);
	}
	
	template <typename T>
	void extractIcon(const char* exe, T &callback)
	{
		UTIL::CB::TemplateCallback<T> c(callback);
		extractIconCB(exe, &c);
	}	

	void extractIconCB(const char* exe, UTIL::CB::CallbackI* callback);

	//! Removes bad chars in file names
	std::string sanitiseFileName(const char* name);

	//! Uses explorer run as command to launch an item
	//!
	bool runAs(const char* command, const char* area = "open");
}
}

#endif